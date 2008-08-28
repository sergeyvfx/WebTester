/*
 *
 * ================================================================================
 *  ipc.c - part of WebTester Server
 * ================================================================================
 *
 *  IPC core stuff module
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "core.h"
#include "ipc.h"
#include "sock.h"
#include "sock.h"
#include "cmd.h"
#include "smartinclude.h"
#include <malloc.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

cmd_context_t *ipc_cmd_context;

int ipc_inet=-1;
ipc_client_t *ipc_clients=0;
int ipc_client_connected=0;
int ipc_max_client_count=IPC_MAX_CLIENT_COUNT;
ipc_client_t *ipc_current_client=0;

static char in_buf[65535];

#define IPC_PROC_REGISTER(__name,__entry_point) \
  cmd_context_proc_register (ipc_cmd_context, __name, __entry_point);

static int
ipc_init_clients                   (void)
{
  int i;
  ipc_clients=malloc (ipc_max_client_count*sizeof (ipc_client_t));
  ipc_client_connected=0;
  for (i=0; i<ipc_max_client_count; i++)
    {
      ipc_clients[i].sock=-1;
      ipc_clients[i].id=i;
      ipc_clients[i].cmd[0]=0;
      ipc_clients[i].authontificated=0;
    }
  return 0;
}

static int
ipc_done_clients                   (void)
{
  int i;
  if (!ipc_clients) return -1;
  for (i=0; i<ipc_max_client_count; i++)
    {
      if (ipc_clients[i].sock>=0)
        {
          sock_answer (ipc_clients[i].sock, "The system is SHUTTING DOWN.\n");
          ipc_disconnect_client (&ipc_clients[i], 1);
////////////////////////////////////////////
// But WHY it works correcly???
////////////////////////////////////////////
//          sock_destroy (ipc_clients[i].sock);
//          ipc_clients[i].sock=-1;
        }
    }
  ipc_client_connected=0;
  free (ipc_clients);
  return 0;
}

static int
ipc_builtin_init                   ()
{
  IPC_PROC_REGISTER ("exit", ipc_proc_exit);
  return 0;
}

int
ipc_init                           (char *__host, unsigned int __port)
{
  ipc_init_clients ();
  if ((ipc_inet=sock_create_inet (__host, __port))<0)
    return -1;
  listen (ipc_inet, 0);
  sock_set_nonblock (ipc_inet, 1);
  ipc_cmd_context=cmd_create_context ("IPC_CONTEXT");
  ipc_builtin_init ();
  return 0;
}

int
ipc_done                           (void)
{
  ipc_done_clients ();
  if (ipc_inet>=0)	
    sock_destroy (ipc_inet);
  ipc_inet = -1;
  if (ipc_cmd_context)
    cmd_destroy_context (ipc_cmd_context);
  return 0;
}

static int
ipc_init_client                    (int __sock)
{
  int i;
  if (ipc_client_connected>ipc_max_client_count)
    {
      sock_destroy (__sock);
      core_print (MSG_INFO, "IPC: Client not connected (limit by max connected users).\n");
      return -1;
    }
  for (i=0; i<ipc_max_client_count; i++)
    {
      if (ipc_clients[i].sock<0)
        {
          core_print (MSG_INFO, "IPC: Client connected (#%d).\n", i);
          ipc_clients[i].sock=__sock;
          sock_set_nonblock (__sock, 1);
          sock_answer (__sock, "Ok\n------------------------\n%s IPC interface greets you.\nYou must authentificate you to restricted stuff.\nType `help' for help :)\n------------------------\n\n", PACKAGE_NAME);
          break;
        }
    }
  return 0;
}

int
ipc_listen                         (void)
{
  int acc_s;
  struct sockaddr addr;
  socklen_t len;
  if (ipc_inet<0) return -1;
  len=sizeof (addr);
  if ((acc_s=accept (ipc_inet, &addr, &len))>=0)
    {
      ipc_init_client (acc_s);
    }
  return 0;
}

static int
ipc_parse_client                   (ipc_client_t *__self)
{
  int argc=0;
  char **argv=0;
  if (__self->cmd[0]==0) return 0;
  cmd_parse_buf (__self->cmd, &argv, &argc);
  ipc_current_client=__self;
  if (argc>0)
    if (cmd_context_execute_proc (ipc_cmd_context, argv, argc))
      sock_answer (__self->sock, "Command unknown: `%s'\n", argv[0]);
  cmd_free_arglist (argv, argc);
  __self->cmd[0]=0;
  return 0;
}

static int
ipc_parse_clients                  ()
{
  int i;
  for (i=0; i<ipc_max_client_count; i++)
    {
      if (ipc_clients[i].sock>=0)
        ipc_parse_client (&ipc_clients[i]);
    }
  return 0;
}

int
ipc_interact                       (void)
{
  int i, len;
  for (i=0; i<ipc_max_client_count; i++)
    {
      if (ipc_clients[i].sock>=0)
        {
          len=recv (ipc_clients[i].sock, in_buf, 65535, 0);
          if (len<0)
            {
              if (errno!=EWOULDBLOCK)
                {
                  // Connection to client losted :(
                  ipc_disconnect_client (&ipc_clients[i], 0);
                }
            } else
            if (len!=0) // Socket is non-blocking, so `0' means error - socket closed by foreign host
              {
                in_buf[len]=0;
                strcpy (ipc_clients[i].cmd, in_buf);
              } else
              {
                // Connection to client losted :(
                ipc_disconnect_client (&ipc_clients[i], 0);
              }
        }
    }
  // Parse and execute users' commands
  ipc_parse_clients ();
  return 0;
}

int
ipc_disconnect_client              (ipc_client_t *__self, int __send_info)
{
  if (__self->sock>=0)
    {
      if (__send_info)
        sock_answer (__self->sock, "You have been disconnected from server.\n");
      sock_destroy (__self->sock);
      __self->authontificated=0;
      core_print (MSG_INFO, "IPC: Client diconnected (#%d).\n", __self->id);
      __self->sock=-1;
    }
  return 0;
}

ipc_client_t *
ipc_get_current_client             (void)
{
  return ipc_current_client;
}

int
ipc_proc_register                  (char *__procname, cmd_entry_point __entrypoint)
{
  IPC_PROC_REGISTER (__procname, __entrypoint);
  return 0;
}
