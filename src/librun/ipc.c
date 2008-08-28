/*
 * ================================================================================
 *  ipc.c - part of the LibRUN
 * ================================================================================
 *
 * IPC interface for LibRUN
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "ipc.h"

#include <libwebtester/smartinclude.h>
#include <libwebtester/conf.h>
#include <libwebtester/sock.h>
#include <libwebtester/cmd.h>
#include <libwebtester/util.h>
#include <libwebtester/log.h>

#include <errno.h>
#include <stdarg.h>
#include <malloc.h>

#define BUFSIZE 65535

////////
// Some type defeninitns

typedef struct {
  char             name[64];
  cmd_entry_point  proc;
} ipc_callback_t;

////////
//

static ipc_callback_t callbacks[]= {
  {"unique",     run_ipc_proc_unique},
  {"security",   run_ipc_proc_security},
  {"taskpid",    run_ipc_proc_taskpid},
  {"finalize",   run_ipc_proc_finalize},
  {"exit_code",  run_ipc_proc_exit_code},
  {"termsig",    run_ipc_proc_termsig},
  {"stopsig",    run_ipc_proc_stopsig},
  {"continue",   run_ipc_proc_continue},
  {"base",       run_ipc_proc_base},
  {"exec_error", run_ipc_proc_exec_error},
  {"validate",   run_ipc_proc_validate},
  {"", 0}
};

static int  sock = -1;
static long port = RUN_IPC_PORT;

static char host[128];   // Server host
static char chost[128];  // Client host

////////////////////////////////////////
// Clients' stuff

// All rgistered IPC clients
static run_ipc_client *clients=NULL;
// Count of registered IPC clients
static int            registered_clients=0;
// Ptr to current client
static run_ipc_client *current_client=0;

// MAX count of registered clients
static int max_clients=RUN_IPC_MAX_CLIENTS;

// Command context
static cmd_context_t *cmd_context=0;

static double max_keep_timeout = RUN_IPC_MAX_KEEP_TIMEOUT;

////
//

static void     // Disconnect client from IPC
disconnect_client                  (run_ipc_client *__self);

////
//

static int      // Initialize clients' stuff
init_clients                       (void)
{
  int i;

  clients=malloc (sizeof (run_ipc_client)*max_clients);

  for (i=0; i<max_clients; i++)
    {
      clients[i].num    = i;
      clients[i].sock   = -1;
      clients[i].unique = -1;
      memset (clients[i].security_key, 0, sizeof (clients[i].security_key));
    }

  registered_clients = 0;
  current_client     = 0;

  return 0;
}

static void     // Initialize client info struct
spawn_new_client                   (run_ipc_client *__self, int __sock)
{
  __self->sock      = __sock;
  __self->unique    = -1;
  __self->timestamp = now ();
  memset (__self->security_key, 0, sizeof (__self->security_key));
}

static int     // Register new client
register_client                    (int __sock)
{
  int i;
  timeval_t cur_time=now ();

  DEBUG_LOG ("librun", "IPC: registering new client...\n");

  // Some optimization :)
  if (registered_clients>=max_clients) return -1;
  for (i=0; i<max_clients; i++)
    {
      if (clients[i].sock>=0 && CHECK_TIME_DELTA (clients[i].timestamp, cur_time, max_keep_timeout*USEC_COUNT))
        {
          disconnect_client  (&clients[i]);
        }

      if (clients[i].sock<0)
        {
          DEBUG_LOG ("librun", "IPC: Client registered to cell %d\n", i);
          spawn_new_client (&clients[i], __sock);
          registered_clients++;
          return i;
        }
    }
  DEBUG_LOG ("librun", "IPC: Failed registering client\n");
  return -1;
}

static void     // Disconnect client from IPC
disconnect_client                  (run_ipc_client *__self)
{
  if (!__self || __self->sock<0) return;

  DEBUG_LOG ("librun", "IPC: Disconnecting client %d\n", __self->num);

  sock_destroy (__self->sock);
  __self->sock   = -1;
  __self->unique = -1;
  memset (__self->security_key, 0, sizeof (__self->security_key));
  registered_clients--;

  DEBUG_LOG ("librun", "IPC: Client disconnected\n");

  //
  // TODO:
  //  Add task terminating stuff here
  //
  //  But are we need this?
  //

}

static void     // Uninitialize IPC stuff
done_clients                       (void)
{
  int i;
  if (!clients) return;
  for (i=0; i<max_clients; i++)
    disconnect_client (&clients[i]);
  free (clients);
}

static void     // Parse command of specified client
parse_client                       (run_ipc_client *__self)
{
  static char cur_cmd[RUN_SOCK_STACK_SIZE];
  int argc=0, i=0, len, clen;
  char **argv=0;
  run_process_info_t *proc;

  if (__self->cmd[0]==0) return;

  DEBUG_LOG ("librun", "IPC: Parsing command `%s` from client %d...\n", __self->cmd, __self->num);

  // Update last action info
  __self->timestamp=now ();
  
  // If process is locked, the safest way not to execute command
  if (__self->unique>=0)
    {
      proc=run_process_info_by_unique (__self->unique);
      if (proc && RUN_PROC_LOCKED (*proc))
        {
          DEBUG_LOG ("librun", "IPC: Parsing command  from client %d canceled: process is locked\n", __self->num);
          return;
        }
    }

  // Some initialization
  current_client=__self;
  len=strlen (__self->cmd);

  while (i<len)
    {
      // Collect one IPC command
      clen=0;
      while (__self->cmd[i]!='\n' && i<len)
        cur_cmd[clen++]=__self->cmd[i++];
      cur_cmd[clen]=0;

      // Parsing and executing
      cmd_parse_buf (cur_cmd, &argv, &argc);
      if (argc>0)
        cmd_context_execute_proc (cmd_context, argv, argc);
      cmd_free_arglist (argv, argc);
      i++;
    }

  __self->cmd[0]=0;
  DEBUG_LOG ("librun", "IPC: Finished parsing command  from client %d\n");
}

static void     // Parse command buffers of all users
parse_clients                      (void)
{
  int i;

  for (i=0; i<max_clients; i++)
    if (clients[i].sock>=0)
      parse_client (&clients[i]);
  current_client=0;
}

////////
//

int
run_ipc_listen                     (void)
{
  int acc_s;
  struct sockaddr addr;
  socklen_t len;
  if (sock<0) return -1;
  len=sizeof (addr);
  if ((acc_s=accept (sock, &addr, &len))>=0)
    {
      if (register_client (acc_s)<0)
        {
          sock_answer (acc_s, "-ERR\n");
          sock_destroy (acc_s);
        } else
        {
          sock_answer (acc_s, "+OK\n");
          sock_set_nonblock (acc_s, TRUE);
        }
    }
 return 0;
}

int
run_ipc_interact                   (void)
{
  int i, len;
  static char in_buf[65536];

  for (i=0; i<max_clients; i++)
    {
      if (clients[i].sock>=0)
        {
          len=recv (clients[i].sock, in_buf, 65535, 0);

          if (len<0)
            {
              if (errno!=EWOULDBLOCK)
                {
                  // Connection to client losted :(
                  disconnect_client (&clients[i]);
                }
            } else
            if (len!=0) // Socket is non-blocking, so `0' means error - socket closed by foreign host
              {
                in_buf[len]=0;
                strcat (clients[i].cmd, in_buf);
              } else
              {
                // Connection to client losted :(
                disconnect_client (&clients[i]);
              }
        }
    }
  // Parse and execute users' commands
  parse_clients ();
  return 0;
}

////////////////////////////////////////
// SOCK built-in

static void     // Read some data from config file
read_config                        (void)
{
  // Get parameters from config file
  CONFIG_INT_KEY (max_clients, "LibRUN/IPC/MaxClients");
  CONFIG_INT_KEY (port,        "LibRUN/IPC/Port");

  CONFIG_FLOAT_KEY (max_keep_timeout,  "LibRUN/IPC/MaxKeepTimeout");
  
  RESET_LEZ (max_clients,      RUN_IPC_MAX_CLIENTS);
  RESET_LEZ (port,             RUN_IPC_PORT);
  RESET_LEZ (max_keep_timeout, RUN_IPC_MAX_KEEP_TIMEOUT);

  strcpy (host,  RUN_IPC_HOST);
  strcpy (chost, RUN_IPC_CLIENT_HOST);
  CONFIG_PCHAR_KEY (host,  "LibRUN/IPC/Host");
  CONFIG_PCHAR_KEY (chost, "LibRUN/LRVM/Host");
}

BOOL            // Initialize IPC stuff
run_ipc_init                       (void)
{
  int i;
  read_config ();

  // Initializing cmd context
  cmd_context=cmd_create_context ("LibRUN IPC Contest");

  i=0;
  while (callbacks[i].proc)
    {
      run_ipc_register_proc (callbacks[i].name, callbacks[i].proc);
      i++;
    }

  init_clients ();

  sock=sock_create_inet (host, port);

  if (sock<0)
    {
      core_set_last_error ("LibRUN: Error create IPC socket");
      return -1;
    }

  sock_set_nonblock (sock, TRUE);
  listen (sock, RUN_SOCK_STACK_SIZE);


  return 0;
}

void            // Uninitialize IPC stuff
run_ipc_done                       (void)
{
  done_clients ();
  sock_destroy (sock);
  cmd_destroy_context (cmd_context);
}

unsigned int    // Return IPC port
run_ipc_port                       (void)
{
  return port;
}

char*           // Return IPC host for client
run_ipc_client_host                (void)
{
  return chost;
}

run_ipc_client* // Return current IPC client
run_ipc_current_user               (void)
{
  return current_client;
}

int             // Answer to current client
run_ipc_socket_answer              (char *__text, ...)
{
  char print_buf[BUFSIZE];
  PACK_ARGS (__text, print_buf, BUFSIZE);
  return sock_answer (run_ipc_current_user ()->sock, "%s", print_buf);
}

static int      // Client index by id
run_ipc_cindex_by_unique           (int __unique)
{
  int i;
  for (i=0; i<max_clients; i++)
    if (clients[i].unique==__unique)
      return i;
  return -1;
}

int             // Uninitialize user by unique
run_ipc_unregister_by_unique       (int __unique)
{
  int index;
  index=run_ipc_cindex_by_unique (__unique);
  if (index>=0)
    {
      disconnect_client (&clients[index]);
      return TRUE;
    }
  return FALSE;
}

////////////////////////////////////////
// Built-in stuff

void            // Register new IPC stuff at context
run_ipc_register_proc              (char *__name, cmd_entry_point __entry_point)
{
  cmd_context_proc_register (cmd_context, __name, __entry_point);
}
