/**
 * WebTester Server - server of on-line testing system
 *
 * Pipe plugin for WebTester
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <libwebtester/plugin-defs.h>
#include <libwebtester/core.h>
#include <libwebtester/hook.h>
#include <libwebtester/ipc.h>
#include <libwebtester/conf.h>

#include <webtester/autoinc.h>

#include "pipe.h"

/****
 * type defenitnions
 */

typedef struct
{
  ipc_client_t *ipc_client;

  BOOL registered;
  char uid[128];

} pipe_client_t;

static pipe_client_t clients[IPC_MAX_CLIENT_COUNT];
static BOOL initialized = FALSE;

/**
 * Review pipe clients
 */
static void
review_clients (void)
{
  int i;

  for (i = 0; i < IPC_MAX_CLIENT_COUNT; i++)
    {
      /* Mistmatch between stored and real client's UID */
      if (strcmp (clients[i].ipc_client->uid, clients[i].uid))
        {
          /* Unset unwanted data */
          clients[i].registered = FALSE;

          /* Update data */
          strcpy (clients[i].ipc_client->uid, clients[i].uid);
        }
    }
}

/**
 * Get client by IPC info
 *
 * @param __info - info to get client by
 * @return client descriptor
 */
static pipe_client_t*
get_client_by_ipc_info (ipc_client_t *__info)
{
  review_clients ();
  return &clients[__info->id];
}

/**
 * Handler of IPC command `pipe`
 *
 * @param __argc - count of arguments
 * @param __argv - argument values
 * @return zero on success, non-zero otherwise
 */
static int
ipc_pipe (int __argc, char **__argv)
{
  pipe_client_t *client = get_client_by_ipc_info (ipc_get_current_client ());

  IPC_ADMIN_REQUIRED

  if (__argc == 2)
    {
      if (!strcmp (__argv[1], "help"))
        {
          IPC_PROC_ANSWER ("+OK Usage: pipe [register|unregister]\n"
                           "`stat register` regsiters you as client to "
                           "receive all messages from CORE\n"
                           "`stat unregister` makes opposite action.\n");
        }
      else if (!strcmp (__argv[1], "register"))
        {
          if (!client->registered)
            {
              client->registered = TRUE;
              IPC_PROC_ANSWER ("+OK\n");
            }
          else
            {
              IPC_PROC_ANSWER ("-ERR You have been already "
                               "registered as PIPE client\n");
            }
        }
      else if (!strcmp (__argv[1], "unregister"))
        {
          if (client->registered)
            {
              client->registered = FALSE;
              IPC_PROC_ANSWER ("+OK\n");
            }
          else
            {
              IPC_PROC_ANSWER ("-ERR You isn't PIPE client\n");
            }
        }
      else
        {
          goto __usage_;
        }
    }
  else
    {
      goto __usage_;
    }

  return 0;

__usage_:
  IPC_PROC_ANSWER ("-ERR Type `pipe help` for help\n");
  return 0;
}

/**
 * Initialize clients
 */
static void
clients_init (void)
{
  int i;
  memset (clients, 0, sizeof (clients));
  for (i = 0; i < IPC_MAX_CLIENT_COUNT; i++)
    {
      clients[i].ipc_client = ipc_get_client_by_id (i);
    }
}

/**
 * Uninitialize clients
 */
static void
clients_done (void)
{
  memset (clients, 0, sizeof (clients));
}

/**
 * Handler of CORE printed info hook
 *
 * @return zero on success, non-zero otherwise
 */
static int
kprint_hook (void *__unused, void *__buffer)
{
  int i;
  for (i = 0; i < IPC_MAX_CLIENT_COUNT; i++)
    {
      if (clients[i].registered)
        {
          sock_answer (clients[i].ipc_client->sock, "+OK %s", __buffer);
        }
    }
  return 0;
}

/**
 * Plugin activation
 *
 * @return zero on success, non-zero otherwise
 */
static int
activate (void *__unused, void *__call_unused)
{
  clients_init ();

  ipc_proc_register ("pipe", ipc_pipe);
  hook_register ("CORE.Print", kprint_hook, 0, HOOK_PRIORITY_NORMAL);

  initialized = TRUE;

  core_print (MSG_INFO, "    **** Pipe CORE->IPC is now enabled\n");

  return 0;
}

/**
 * Plugin deactivation
 *
 * @return zero on success, non-zero otherwise
 */
static int
deactivate (void *__unused, void *__call_unused)
{
  initialized = FALSE;

  hook_unregister ("CORE.Print", kprint_hook, HOOK_PRIORITY_NORMAL);
  clients_done ();
  return 0;
}

/**
 * Initialize oplugin
 *
 * @param __plugin - plugin descriptor
 * @return zero on success, non-zero otherwise
 */
static int
Init (plugin_t *__plugin)
{
  int flag = 0;

  CONFIG_BOOL_KEY (flag, "Server/Plugins/pipe/Enabled");
  if (flag)
    {
      hook_register (CORE_ACTIVATE, activate, 0, HOOK_PRIORITY_NORMAL);
      hook_register (CORE_DEACTIVATE, deactivate, 0, HOOK_PRIORITY_NORMAL);
    }
  return 0;
}

/**
 * Unload oplugin
 *
 * @param __plugin - plugin descriptor
 * @return zero on success, non-zero otherwise
 */
static int
OnUnload (plugin_t *__plugin)
{
  int flag = 0;

  CONFIG_BOOL_KEY (flag, "Server/Plugins/pipe/Enabled");
  if (flag)
    {
      hook_unregister (CORE_ACTIVATE, activate, HOOK_PRIORITY_NORMAL);
      hook_unregister (CORE_DEACTIVATE, deactivate, HOOK_PRIORITY_NORMAL);
    }
  return 0;
}

/****
 * Plugin information struct
 */
static plugin_info_t Info = {
  RUNDLL_MAJOR_VERSION,
  RUNDLL_MINOR_VERSION,

  0,
  OnUnload,

  0,
  0
};

PLUGIN_INIT (RUNDLL_LIBNAME, Init, Info);
