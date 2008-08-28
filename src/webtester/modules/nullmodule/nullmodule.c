/*
 *
 * ================================================================================
 *  nullmodule.c - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <webtester/autoinc.h>
#include <webtester/task.h>

#include <libwebtester/plugin-defs.h>
#include <libwebtester/core.h>
#include <libwebtester/hook.h>

static int
activate                           (void *__unused, void *__call_unused)
{
  return 0;
}

static int
deactivate                         (void *__unused, void *__call_unused)
{
  return 0;
}

////////////////////////////////////////
//

BOOL
StartForTesting                    (wt_task_t *__task, char *__error)
{
  TASK_SET_STATUS (*__task, TS_FINISHED);
  return TRUE;
}

static int
Init                               (plugin_t *__plugin)
{
  hook_register (CORE_ACTIVATE,   activate,   0, HOOK_PRIORITY_NORMAL);
  hook_register (CORE_DEACTIVATE, deactivate, 0, HOOK_PRIORITY_NORMAL);
  return 0;
}

static int
OnUnload                           (plugin_t *__plugin)
{
  hook_unregister (CORE_ACTIVATE,   activate,   HOOK_PRIORITY_NORMAL);
  hook_unregister (CORE_DEACTIVATE, deactivate, HOOK_PRIORITY_NORMAL);
	return 0;
}

////////////////////////
// Plugin registration info

static plugin_info_t Info={
  1,        // Major version
  0,        // Minor version

  0,        // Onload handler
  OnUnload, // OnUnload handler

  0,  // No activation callback
  0   // No deactivation callback
};

PLUGIN_INIT  ("nullmodule", Init, Info);
