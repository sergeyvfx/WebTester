/*
 *
 * ================================================================================
 *  ipc.c - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "autoinc.h"
#include "ipc.h"

#include <libwebtester/hook.h>
#include <libwebtester/ipc.h>
#include <libwebtester/flexval.h>
#include <libwebtester/conf.h>

#include <libwebtester/mutex.h>

#include <glib.h>

static char host[128];
static unsigned int port        = WT_IPC_PORT;
static BOOL         ipc_enabled = FALSE;
static BOOL         activated   = FALSE;

static mutex_t mutex   = NULL;
static GThread *thread = NULL;

static double delay=WT_IPC_DELAY;

////////
//

static gpointer
wt_ipc_mainloop                    (gpointer __unused);

static int      // Start IPC stuff
wt_ipc_start                       (void *__unused, void *__call_unused);

static int      // Stop IPC stuff
wt_ipc_stop                        (void *__unused, void *__call_unused);

////////
//

static void     // Get data from config file
read_config                        (void)
{
  strcpy (host, WT_IPC_HOST);
  CONFIG_PCHAR_KEY (host,  "Server/IPC/Host");
  CONFIG_INT_KEY   (port,  "Server/IPC/Port");
  CONFIG_FLOAT_KEY (delay, "Server/IPC/Delay");

  RESET_LEZ (port,   WT_IPC_PORT);
  RESET_LEZ (delay,  WT_IPC_DELAY);
}

int             // Initialize IPC stuff
wt_ipc_init                        (void)
{
  read_config ();

  if (!ipc_init (host, port))
    ipc_enabled=TRUE;
  hook_register (CORE_ACTIVATE,   wt_ipc_start, 0, HOOK_PRIORITY_NORMAL);
  hook_register (CORE_DEACTIVATE, wt_ipc_stop,  0, HOOK_PRIORITY_NORMAL);
  
  mutex=mutex_create ();

  ipc_proc_register ("help",     wt_ipc_help);
  ipc_proc_register ("login",    wt_ipc_login);
  ipc_proc_register ("logout",   wt_ipc_logout);
  ipc_proc_register ("tail",     wt_ipc_tail);
  ipc_proc_register ("uptime",   wt_ipc_uptime);

  return 0;
}

void            // Uninitialize IPC stuff
wt_ipc_done                        (void)
{
  hook_unregister (CORE_ACTIVATE,   wt_ipc_start, HOOK_PRIORITY_NORMAL);
  hook_unregister (CORE_DEACTIVATE, wt_ipc_stop,  HOOK_PRIORITY_NORMAL);

  ipc_enabled=FALSE;
  ipc_done ();
  mutex_free (mutex);
}

static int      // Start IPC stuff
wt_ipc_start                       (void *__unused, void *__call_unused)
{

  if (ipc_enabled)
    {
      mutex_lock (mutex);
      thread=g_thread_create (wt_ipc_mainloop, 0, TRUE, 0);

      core_print (MSG_INFO, "    **** Connection via IPC ENABLED.\n");
      activated=TRUE;
    }

  return 0;
}

static int      // Stop IPC stuff
wt_ipc_stop                        (void *__unused, void *__call_unused)
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

static gpointer
wt_ipc_mainloop                    (gpointer __unused)
{
  struct timespec timestruc;
  timestruc.tv_sec   = ((unsigned long long)(delay*NSEC_COUNT))/NSEC_COUNT;
  timestruc.tv_nsec  = ((unsigned long long)(delay*NSEC_COUNT))%NSEC_COUNT;

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
