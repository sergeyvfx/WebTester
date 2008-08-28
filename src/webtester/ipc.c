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

static char host[128];
static unsigned int port        = WT_IPC_PORT;
static BOOL         ipc_enabled = FALSE;
static BOOL         activated   = FALSE;

////////
//

static int      // Start IPC stuff
wt_ipc_start                       (void *__unused);

static int      // Stop IPC stuff
wt_ipc_stop                        (void *__unused);

////////
//

static void     // Get data from config file
read_config                        (void)
{
  strcpy (host, WT_IPC_HOST);
  CONFIG_PCHAR_KEY (host, "Server/IPC/Host");
  CONFIG_INT_KEY   (port, "Server/IPC/Port");

  RESET_LEZ (port, WT_IPC_PORT);
}

int             // Initialize IPC stuff
wt_ipc_init                        (void)
{
  read_config ();

  if (!ipc_init (host, port))
    ipc_enabled=TRUE;
  hook_register (CORE_ACTIVATE,   wt_ipc_start, 0, HOOK_PRIORITY_NORMAL);
  hook_register (CORE_DEACTIVATE, wt_ipc_stop,  0, HOOK_PRIORITY_NORMAL);
  return 0;
}

void            // Uninitialize IPC stuff
wt_ipc_done                        (void)
{
  hook_unregister (CORE_ACTIVATE,   wt_ipc_start, HOOK_PRIORITY_NORMAL);
  hook_unregister (CORE_DEACTIVATE, wt_ipc_stop,  HOOK_PRIORITY_NORMAL);

  ipc_enabled=FALSE;
  ipc_done ();
}

static int      // Start IPC stuff
wt_ipc_start                       (void *__unused)
{
  if (ipc_enabled)
    core_print (MSG_INFO, "    **** Connection via IPC ENABLED.\n");
  activated=TRUE;
  return 0;
}

static int      // Stop IPC stuff
wt_ipc_stop                        (void *__unused)
{
  if (!activated) return 0;
  if (ipc_enabled)
    core_print (MSG_INFO, "    **** Connection via IPC DISABLED.\n");
  return 0;
}
