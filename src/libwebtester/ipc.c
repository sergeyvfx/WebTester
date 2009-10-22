/**
 * WebTester Server - server of on-line testing system
 *
 * IPC core stuff module
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "core.h"
#include "ipc.h"
#include "sock.h"
#include "sock.h"
#include "cmd.h"
#include "uid.h"
#include "sock.h"
#include <malloc.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

cmd_context_t *ipc_cmd_context = 0;

int ipc_inet = -1;
ipc_client_t *ipc_clients = 0;
int ipc_client_connected = 0;
int ipc_max_client_count = IPC_MAX_CLIENT_COUNT;
ipc_client_t *ipc_current_client = 0;

static char in_buf[65535];
static int shutdowning = 0;

#define IPC_PROC_REGISTER(__name,__entry_point) \
  if (ipc_cmd_context) \
    cmd_context_proc_register (ipc_cmd_context, __name, __entry_point) \

#define IPC_PROC_UNREGISTER(__name) \
  if (ipc_cmd_context) \
    cmd_context_proc_unregister (ipc_cmd_context, __name) \


/**
 * Init IP clients pool
 *
 * @return zero on succecc, non-zero otherwise
 */
static int
ipc_init_clients (void)
{
  int i;
  ipc_clients = malloc (ipc_max_client_count * sizeof (ipc_client_t));
  ipc_client_connected = 0;

  for (i = 0; i < ipc_max_client_count; i++)
    {
      memset (&ipc_clients[i], 0, sizeof (ipc_clients[i]));
      ipc_clients[i].sock = -1;
      ipc_clients[i].id = i;
    }

  return 0;
}

/**
 * Done all IPC clients
 *
 * @retrun zero on success, non-zero otherwise
 */
static int
ipc_done_clients (void)
{
  int i;
  if (!ipc_clients) return -1;
  for (i = 0; i < ipc_max_client_count; i++)
    {
      if (ipc_clients[i].sock >= 0)
        {
          sock_answer (ipc_clients[i].sock, "The system is SHUTTING DOWN.\n");
          ipc_disconnect_client (&ipc_clients[i], 1);

          /* But WHY it works correcly??? */
          /* sock_destroy (ipc_clients[i].sock);
          ipc_clients[i].sock=-1; */
        }
    }

  ipc_client_connected = 0;
  free (ipc_clients);

  return 0;
}

/**
 * Initialize IPC builtin
 *
 * @return zero on success, non-zero otherwise
 */
static int
ipc_builtin_init (void)
{
  IPC_PROC_REGISTER ("exit", ipc_proc_exit);
  return 0;
}

/**
 * Initialize IPC client
 *
 * @param __sock - client's socket
 * @param __addr - client's addr
 * @return zero on success, non-zero otherwise
 */
static int
ipc_init_client (int __sock, struct sockaddr_in __addr)
{
  int i;

  if (ipc_client_connected > ipc_max_client_count)
    {
      sock_destroy (__sock);
      core_print (MSG_INFO, "IPC: Client not connected "
                             "(limit by max connected users).\n");
      return -1;
    }

  for (i = 0; i < ipc_max_client_count; i++)
    {
      if (ipc_clients[i].sock < 0)
        {
          char *ip = inet_ntoa (__addr.sin_addr);

          core_print (MSG_INFO, "IPC: Client connected from "
                                "IP %s (#%d).\n", ip, i);
          ipc_clients[i].sock = __sock;
          sock_set_nonblock (__sock, 1);

          ipc_clients[i].authontificated = 0;
          ipc_clients[i].access = 0;
          strcpy (ipc_clients[i].login, "");

          ipc_clients[i].timestamp = time (0);
          uid_gen (ipc_clients[i].uid);

          strcpy (ipc_clients[i].ip, ip);

          sock_answer (__sock, "+OK\n------------------------\n"
                               "%s IPC interface greets you.\n"
                               "You must authentificate yourself to get "
                               "access to restricted stuff.\n"
                               "Type `help' for help :)\n"
                               "------------------------\n\n",
                       CORE_PACKAGE_NAME);
          break;
        }
    }

  return 0;
}

/**
 * Parse command from one client
 *
 * @param __self - client whoose command will be parsed
 * @return zero o nsuccess, non-zero otherwise
 */
static int
ipc_parse_client (ipc_client_t *__self)
{
  int argc = 0, n, i, j;
  char **argv = 0, cmd_buf[65536] = {0};

  if (!__self->cmd || __self->cmd[0] == 0)
    {
      return 0;
    }

  /* Skip executing command if client is frozen */
  if (ipc_client_frozen (__self))
    {
      free (__self->cmd);
      __self->cmd = 0;
      return 0;
    }

  strcpy (cmd_buf, __self->cmd);

  i = 0;
  n = strlen (__self->cmd);
  while (__self->cmd[i] < ' ' && i < n)
    {
      i++;
    }

  j = 0;
  for (; i < n; i++)
    {
      if (__self->cmd[i] == '\n' || __self->cmd[i] == '\r')
        {
          break;
        }
      cmd_buf[j++] = __self->cmd[i];
    }
  cmd_buf[j] = 0;

  j = 0;
  for (i++; i < n; i++)
    {
      __self->cmd[j++] = __self->cmd[i];
    }
  __self->cmd[j] = 0;

  LOG ("IPC", "Command from client #%d: %s\n", __self->id, cmd_buf);

  cmd_parse_buf (cmd_buf, &argv, &argc);
  ipc_current_client = __self;
  if (argc > 0)
    {
      if (cmd_context_execute_proc (ipc_cmd_context, argv, argc))
        {
          sock_answer (__self->sock, "-ERR Unknown command: `%s'\n", argv[0]);
        }
    }
  cmd_free_arglist (argv, argc);

  if (__self->cmd)
    {
      free (__self->cmd);
      __self->cmd = 0;
    }

  return 0;
}

/**
 * Parse all clients
 */
static int
ipc_parse_clients (void)
{
  int i;

  for (i = 0; i < ipc_max_client_count; i++)
    {
      if (ipc_clients[i].sock >= 0)
        {
          ipc_parse_client (&ipc_clients[i]);
        }
    }

  return 0;
}

/********
 * User's backend
 */

/**
 * Initialize IPC stuff
 *
 * @param __host - host ot IP address to bind server to
 * @param __port - port number to bind to
 * @return zero on success, non-zero otherwise
 */
int
ipc_init (const char *__host, unsigned int __port)
{
  ipc_init_clients ();

  if ((ipc_inet = sock_create_inet (__host, __port)) < 0)
    {
      return -1;
    }

  listen (ipc_inet, 0);
  sock_set_nonblock (ipc_inet, 1);
  ipc_cmd_context = cmd_create_context ("IPC_CONTEXT");
  ipc_builtin_init ();

  return 0;
}

/**
 * Uninitialize IPC stuff
 */
int
ipc_done (void)
{
  shutdowning = 1;
  ipc_done_clients ();

  if (ipc_inet >= 0)
    {
      sock_destroy (ipc_inet);
    }

  ipc_inet = -1;

  if (ipc_cmd_context)
    {
      cmd_destroy_context (ipc_cmd_context);
      ipc_cmd_context = NULL;
    }
  shutdowning = 0;
  return 0;
}

/**
 * Listen for incoming connections
 *
 * @return zero on success, non-zero otherwise
 */
int
ipc_listen (void)
{
  int acc_s;
  struct sockaddr_in addr;
  socklen_t len;

  if (ipc_inet < 0)
    {
      return -1;
    }

  len = sizeof (addr);
  memset (&addr, 0, sizeof (addr));
  if ((acc_s = accept (ipc_inet, (struct sockaddr*) & addr, &len)) >= 0)
    {
      char *ip = inet_ntoa (addr.sin_addr);

      /* Check if client's IP is blacklisted */
      if (!ipc_blacklisted (ip))
        {
          ipc_init_client (acc_s, addr);
        }
      else
        {
          sock_answer (acc_s, "Your IP has been blacklisted.\n");
          LOG ("IPC", "Tried to connect from blacklisted IP: %s\n", ip);
          sock_destroy (acc_s);
        }
    }

  return 0;
}

/**
 * Execute commands from clients
 *
 * @return zero on success, non-zero otherwise
 */
int
ipc_interact (void)
{
  int i, len;
  for (i = 0; i < ipc_max_client_count; i++)
    {
      if (ipc_clients[i].sock >= 0)
        {
          /* Check if user has been blacklisted */
          if (ipc_blacklisted (ipc_clients[i].ip))
            {
              sock_answer (ipc_clients[i].sock, "Your' IP has been blacklisted.\n");
              ipc_disconnect_client (&ipc_clients[i], 0);
              continue;
            }

          len = recv (ipc_clients[i].sock, in_buf, 65535, 0);
          if (len < 0)
            {
              if (errno != EWOULDBLOCK)
                {
                  /* Connection to client losted :( */
                  ipc_disconnect_client (&ipc_clients[i], 0);
                }
            }
          else
            {
              if (len != 0)
                {
                  /* Socket is non-blocking, */
                  /* so `0' means error - socket closed by foreign host */
                  int cmd_len = 0;
                  char *scmd = 0;
                  in_buf[len] = 0;
                  cmd_len = len;

                  if (ipc_clients[i].cmd)
                    cmd_len += strlen (ipc_clients[i].cmd);

                  scmd = ipc_clients[i].cmd;
                  ipc_clients[i].cmd = malloc (cmd_len + 1);
                  strcpy (ipc_clients[i].cmd, "");
                  if (scmd)
                    {
                      strcpy (ipc_clients[i].cmd, scmd);
                      free (scmd);
                    }
                  strcat (ipc_clients[i].cmd, in_buf);
                }
              else
                {
                  /* Connection to client losted :( */
                  ipc_disconnect_client (&ipc_clients[i], 0);
                }
            }
        }
    }

  /* Parse and execute users' commands */
  ipc_parse_clients ();
  return 0;
}

/**
 * Disconnect client from server
 *
 * @param __self - client to disconnect
 * @param __send_info - send information to client he's diconnected
 */
int
ipc_disconnect_client (ipc_client_t *__self, int __send_info)
{
  if (__self->sock >= 0)
    {
      if (__send_info)
        {
          sock_answer (__self->sock,
                       "You have been disconnected from server.\n");
        }

      sock_destroy (__self->sock);
      __self->authontificated = 0;
      __self->access = 0;
      strcpy (__self->login, "");
      strcpy (__self->uid, "");

      if (!shutdowning)
        {
          /* For more sucky beaut y*/
          core_print (MSG_INFO, "IPC: Client disconnected (#%d).\n",
                      __self->id);
        }

      __self->sock = -1;
    }
  return 0;
}

/**
 * Get currently executing client
 *
 * @return current client's descriptor
 */
ipc_client_t*
ipc_get_current_client (void)
{
  return ipc_current_client;
}

/**
 * Get client by it's ID
 *
 * @return client with needed ID
 */
ipc_client_t *
ipc_get_client_by_id (int __id)
{
  return &ipc_clients[__id];
}

/**
 * Register procedure in IPC command context
 *
 * @param __procname - name of procedure
 * @param __entrypoint - entry point to handler of this procedure
 * @return zero on success, non-zero otherwise
 */
int
ipc_proc_register (const char *__procname, cmd_entry_point __entrypoint)
{
  IPC_PROC_REGISTER (__procname, __entrypoint);
  return 0;
}

/**
 * Unregister procedure from IPC command context
 *
 * @param __procname - name of procedure to unregister
 * @return zero on success, non-zero otherwise
 */
int
ipc_proc_unregister (const char *__procname)
{
  IPC_PROC_UNREGISTER (__procname);
  return 0;
}

/****
 * Freezing stuff
 */

/**
 * Is IPC client forzen?
 *
 * @param __self - client to check
 * @return zero if client is not forizen, non-zero otherwise
 */
int
ipc_client_frozen (ipc_client_t *__self)
{
  if (__self->flags & IPCCF_FROZEN)
    {
      if (tv_usec_cmp (timedist (__self->frozen_timestamp, now ()),
                       __self->freeze_duration * 1000) > 0)
        {
          __self->flags &= ~IPCCF_FROZEN;
          return 0;
        }
      return 1;
    }

  return 0;
}

/**
 * Freeze IPC client
 *
 * @param __self - client to be frozen
 * @param __free duration
 */
void
ipc_client_freeze (ipc_client_t *__self, int __duration)
{
  __self->flags |= IPCCF_FROZEN;
  __self->frozen_timestamp = now ();
  __self->freeze_duration = __duration;
}
