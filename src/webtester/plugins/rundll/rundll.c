/*
 *
 * ================================================================================
 *  rundll.c - part of the WebTester Server
 * ================================================================================
 *
 *  This plugin is needed only for initialization/ubinitialization stuff
 *  for LibRUN.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <libwebtester/plugin-defs.h>
#include <libwebtester/core.h>
#include <libwebtester/hook.h>

#include <webtester/autoinc.h>
#include <librun/run.h>

#include "rundll.h"

static int
activate                           (void *__unused)
{
  // Initialize profiling stuff

  if (run_init ())
    {
      core_print (MSG_INFO, "    **** Error initializing LibRUN. Profiling is UNAVALIABLE.\n");
      return -1;
    }
  core_print (MSG_INFO, "    **** LibRUN profiling library is now activated.\n");
  return 0;
}

static int
deactivate                         (void *__deactivate)
{
  run_done ();
  core_print (MSG_INFO, "    **** Profiling throught LibRUN is now unavaliable.\n");
  return 0;
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
//

static plugin_info_t Info={
  RUNDLL_MAJOR_VERSION,
  RUNDLL_MINOR_VERSION,

  0,
  OnUnload
};

PLUGIN_INIT  (RUNDLL_LIBNAME, Init, Info);
