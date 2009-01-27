/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "ipc.h"

#include <libwebtester/hook.h>
#include <libwebtester/ipc.h>
#include <libwebtester/flexval.h>
#include <libwebtester/conf.h>

#include <libwebtester/mutex.h>

#include <glib.h>

static char host[128] = {0};
static char blacklist_file[4096] = {0};
static unsigned int port = WT_IPC_PORT;
static BOOL ipc_enabled = FALSE;
static BOOL activated = FALSE;
static BOOL blacklisting = FALSE;
static BOOL blacklisting_error = FALSE;

static mutex_t mutex = NULL;
static GThread *thread = NULL;

static double delay = WT_IPC_DELAY;

static long reset_timeout = WT_IPC_BLACKLIST_RESET_TIMEOUT;

/**
 * Get data from config file
 */
static void
read_config (void)
{
  char dummy[1024];

  strcpy (host, WT_IPC_HOST);
  CONFIG_PCHAR_KEY (host, "Server/IPC/Host");
  CONFIG_INT_KEY (port, "Server/IPC/Port");
  CONFIG_FLOAT_KEY (delay, "Server/IPC/Delay");

  CONFIG_INT_KEY (reset_timeout, "Server/IPC/Blacklisting/ResetTimeout");

  CONFIG_PCHAR_KEY (dummy, "Server/IPC/Blacklisting/Enabled");
  blacklisting = is_truth (dummy);

  CONFIG_PCHAR_KEY (blacklist_file, "Server/IPC/Blacklisting/BlacklistFile");

  RESET_LEZ (port, WT_IPC_PORT);
  RESET_LEZ (delay, WT_IPC_DELAY);
}

static gpointer
wt_ipc_mainloop (gpointer __unused)
{
  struct timespec timestruc;
  timestruc.tv_sec = ((unsigned long long) (delay * NSEC_COUNT)) / NSEC_COUNT;
  timestruc.tv_nsec = ((unsigned long long) (delay * NSEC_COUNT)) % NSEC_COUNT;

  for (;;)
    {
      if (mutex_trylock (mutex))
        {
          mutex_unlock (mutex);
          break;
        }

      ipc_listen ();
      ipc_interact ();

      nanosleep (&timestruc, 0);
    }

  g_thread_exit (0);
  return 0;
}

/**
 * Start IPC stuff
 */
static int
wt_ipc_start (void *__unused, void *__call_unused)
{

  if (blacklisting_error)
    {
      core_print (MSG_INFO, "    **** Error while initializing blacklisting.\n");
      core_print (MSG_INFO, "    **** Connection via IPC DISABLED.\n");
    }

  if (ipc_enabled)
    {
      mutex_lock (mutex);
      thread = g_thread_create (wt_ipc_mainloop, 0, TRUE, 0);

      core_print (MSG_INFO, "    **** Connection via IPC ENABLED.\n");
      activated = TRUE;
    }

  return 0;
}

static int // Stop IPC stuff
wt_ipc_stop (void *__unused, void *__call_unused)
{
  if (!activated) return 0;
  if (ipc_enabled)
    {
      mutex_unlock (mutex);
      g_thread_join (thread);
      core_print (MSG_INFO, "    **** Connection via IPC DISABLED.\n");
    }
  return 0;
}

/********
 * User backend
 */

/**
 * Initialize IPC stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
wt_ipc_init (void)
{
  read_config ();

  if (blacklisting)
    {
      if (ipc_blacklist_init (blacklist_file, reset_timeout))
        {
          blacklisting_error = TRUE;
          return -1;
        }
    }

  if (ipc_init (host, port))
    {
      return -1;
    }

  ipc_enabled = TRUE;

  hook_register (CORE_ACTIVATE, wt_ipc_start, 0, HOOK_PRIORITY_NORMAL);
  hook_register (CORE_DEACTIVATE, wt_ipc_stop, 0, HOOK_PRIORITY_NORMAL);

  mutex = mutex_create ();

  wt_ipc_builtin_init ();

  return 0;
}

/**
 * Uninitialize IPC stuff
 */
void
wt_ipc_done (void)
{
  ipc_enabled = FALSE;

  wt_ipc_builtin_done ();

  hook_unregister (CORE_ACTIVATE, wt_ipc_start, HOOK_PRIORITY_NORMAL);
  hook_unregister (CORE_DEACTIVATE, wt_ipc_stop, HOOK_PRIORITY_NORMAL);

  ipc_done ();
  mutex_free (mutex);
}

/**
 * Check is IPC supported
 *
 * @return non-zero in case IPC is supported, zero otherwise
 */
int
wt_ipc_supported (void)
{
  return ipc_enabled;
}
