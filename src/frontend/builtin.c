/*
 *
 * ================================================================================
 *  builtin.h - part of the WebTester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "common.h"
#include "builtin.h"
#include "console.h"
#include "stat.h"
#include "iface.h"
#include "pipe.h"

#include <libwebtester/sock.h>
#include <libwebtester/conf.h>

#include <string.h>
#include <stdlib.h>

////
//

#define C_ARG_EQ(__s)  (!strcmp (__argv[i], __s))
#define CHECK_NEXT     (i<__argc-1)
#define NEXT_ARG       (__argv[++i])

////
//

static BOOL connected  = FALSE;
static BOOL connecting = FALSE, disconnecting = FALSE;

static char config_file[4096]=CONFIG_FILE;

static char btncmds[BUTTONS_COUNT][4096]={{0}};

////
//

static int
create_socket                      (char *__host, __u32_t __port)
{
  int sock;
  if ((sock=sock_create_client_inet (__host, __port))<0)
    {
      switch (sock)
        {
        case SE_UNKNOWN_HOST:
          console_log ("IPC: Hostname is unsearchable\n");
          break;
        case SE_CANT_CREATE_SOCK:
          console_log ("IPC: Can not create socket\n");
          break;
        case SE_CANT_CONNECT:
          console_log ("IPC: Can not connect socket to server\n");
          break;
        }
      return -1;
    }
  sock_set_nonblock (sock, FALSE);
  return sock;
}

static BOOL
socket_login                       (int __sock, char *__login, char *__pass, char *__ans)
{
  char cmd[1024];
  ipc_ans_t ans;
  
  sprintf (cmd, "login %s %s", __login, __pass);
  
  sock_answer (__sock, "%s\n", cmd);
  memset (cmd, 0, sizeof (cmd));

  if (__ans)
    strcpy (__ans, "");

  if (read_ipc_sock (__sock, &ans))
    return FALSE;

  if (__ans)
    strcpy (__ans, IPC_ANS_BODY (ans));

  return ans.type==IAT_OK;
}

static BOOL
create_sockets                     (char *__host, __u32_t __port, BOOL __login_at_connect, char *__login, char *__pass)
{
  BOOL logged=FALSE;
  int sock;
  ipc_ans_t ans;
  char res[4096];

  // Create socket for user's console
  sock=create_socket (__host, __port);
  if (sock<0) return FALSE;
  read_ipc_sock (sock, &ans);
  console_log ("%s", IPC_ANS_BODY (ans));
  if (__login_at_connect)
    if (!(logged=socket_login (sock, __login, __pass, res)))
      console_log ("%s", res);
  console_set_socket (sock);

  // Create socket for stat

  //
  // TODO:
  //  But maybe it'll be better if we create this socket only if we can get
  //  statistics info?
  //

  sock=create_socket (__host, __port);
  if (sock<0) return FALSE;
  read_ipc_sock (sock, &ans);
  if (__login_at_connect && logged) socket_login (sock, __login, __pass, 0);
  stat_set_socket (sock);

  // Create socket for pipe
  if (__login_at_connect && logged) {
    sock=create_socket (__host, __port);
    if (sock<0) return FALSE;
    read_ipc_sock (sock, &ans);
    socket_login (sock, __login, __pass, 0);
    pipe_set_socket (sock);
  }

  connected=TRUE;

  return TRUE;
}

static void
bind_buttons                       (void)
{
  int i;
  char path_prefix[1024], cpt[1024], cmd[4096], path[1024];

  for (i=0; i<BUTTONS_COUNT; i++)
    {
      sprintf (path_prefix, "Client/IFACE/Buttons/%d", i);
      if (CONFIG_KEY_EXISTS (path_prefix))
        {
          strcpy (cpt, "");
          strcpy (cmd, "");

          sprintf (path, "%s/Caption", path_prefix);
          CONFIG_PCHAR_KEY (cpt, path);

          sprintf (path, "%s/Command", path_prefix);
          CONFIG_PCHAR_KEY (cmd, path);

          bind_button (i, cpt, cmd);
        }
    }
}

static void
on_post_config_load                (void)
{
  char host[1024]={0}, port[1024]={0}, dummy[1024];

  CONFIG_PCHAR_KEY (host, "Client/IPC/Host");
  CONFIG_PCHAR_KEY (port, "Client/IPC/Port");

  if (strcmp (host, "") && strcmp (port, ""))
    {
      sprintf (dummy, "%s:%s", host, port);
      gtk_entry_set_text ((GtkEntry*)lookup_widget (main_window, "server"), dummy);
    }
  
  bind_buttons ();
}

////////
//

BOOL
parse_args                         (int __argc, char **__argv)
{
  int i;
  for (i=0; i<__argc; i++)
    {
      if (C_ARG_EQ ("--config-file") || C_ARG_EQ ("-c"))
        {
          if (CHECK_NEXT)
            strcpy (config_file, NEXT_ARG);
        }
    }
  return TRUE;
}

BOOL
load_config_file                   (void)
{
  // Loading config file
  if (config_init (config_file))
    {
      _ERROR ("Error loading config file: %s\n", config_get_error ());
      return FALSE;
    }

  on_post_config_load ();

  return TRUE;
}

BOOL
connect_to_server                  (char *__server, BOOL __login_at_connect, char *__login, char *__pass)
{
  char host[4096], dummy[1024];
  int i=0, len=0, n=strlen (__server);
  __u32_t port;

  if (connecting || connected)
    return TRUE;

  connecting=TRUE;

  // Collect hostname of server
  for (i=0; i<n; i++)
    {
      if (__server[i]==':')
        {
          i++;
          break;
        }
      host[len++]=__server[i];
    }
  host[len]=0;

  // Collect port
  len=0;
  for (; i<n; i++)
    dummy[len++]=__server[i];
  dummy[len]=0;
  port=atol (dummy);

  if (create_sockets (host, port, __login_at_connect, __login, __pass))
    {
      iface_set_widgets_connected (TRUE);
      connecting=FALSE;
      return TRUE;
    }

  connecting=FALSE;

  return FALSE;
}

void
disconnect_from_server             (void)
{
  if (!connected || disconnecting)
    return;

  disconnecting=TRUE;

  sock_destroy (console_get_socket ());
  sock_destroy (stat_get_socket ());
  sock_destroy (pipe_get_socket ());
  console_set_socket (-1);
  stat_set_socket (-1);
  pipe_set_socket (-1);

  if (connected)
    console_log ("Disconnected from server.\n");

  iface_set_widgets_connected (FALSE);
  stat_on_disconnect ();

  connected=FALSE;
  disconnecting=FALSE;
}

int
read_ipc_sock                      (int __sock, ipc_ans_t *__ans)
{
  __ans->type=IAT_UNKNOWN;
  strcpy (__ans->body.data, "");
  if (!__ans || __sock<0 || (!connected && !connecting)) return -1;

  __ans->body.length=sock_read (__sock, 65536, __ans->body.data);

  // Because of sockets aren't blockable
  if (IPC_ANS_LEN (*__ans)<=0)
    {
      disconnect_from_server ();
      return -1;
    }

  __ans->body.data[__ans->body.length]=0;

  if (!strncmp (__ans->body.data, "+OK", 3))
    __ans->type=IAT_OK; else
  if (!strncmp (__ans->body.data, "+ERR", 4))
    __ans->type=IAT_ERR;

  return 0;
}

void
bind_button                        (int __i, char *__cpt, char *__cmd)
{
  GtkButton *btn;
  char tmp[64];

  if (__i<0 || __i>=BUTTONS_COUNT)
    return;

  sprintf (tmp, "ctrlButton_%d", __i);
  btn=(GtkButton*)lookup_widget (main_window, tmp);

  gtk_button_set_label (btn, __cpt);

  strcpy (btncmds[__i], __cmd);
}

void
exec_button_command                (int __i)
{
  if (__i<0 || __i>=BUTTONS_COUNT)
    return;
  
  console_cmd_send_specified (btncmds[__i]);
}
