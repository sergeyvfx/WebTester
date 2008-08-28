/*
 *
 * ================================================================================
 *  informaticse.c - part of the WebTester Server
 * ================================================================================
 *
 *  Testing module for simple olympiands.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "informatics.h"
#include "macros.h"

#include <libwebtester/plugin-defs.h>
#include <libwebtester/core.h>
#include <libwebtester/hook.h>
#include <libwebtester/scheduler.h>

static int
activate                           (void *__unused)
{
  Informatics_init_testing ();
  return 0;
}

static int
deactivate                         (void *__deactivate)
{
  Informatics_done_testing ();
  return 0;
}

////////////////////////////////////////
//

BOOL
StartForTesting                    (wt_task_t *__task, char *__error)
{
  if (!Informatics_start_testing_thread (__task, __error))
    return FALSE;
  return TRUE;
}

static int
Init                               (plugin_t *__plugin)
{
  double checker_upload_interval;

  hook_register (CORE_ACTIVATE,    activate,                 0, HOOK_PRIORITY_NORMAL);
  hook_register (CORE_DEACTIVATE,  deactivate,               0, HOOK_PRIORITY_NORMAL);
  hook_register (CORE_STOPTESTING, Informatics_stop_testing, 0, HOOK_PRIORITY_NORMAL);

  hook_register (CORE_STOPTESTING, Informatics_StopCheckerUploading, 0, HOOK_PRIORITY_NORMAL);
  hook_register (CORE_STOPTESTING, Informatics_StopProblemUploading, 0, HOOK_PRIORITY_NORMAL);

  hook_register (CORE_UPLOADPROBLEMS, Informatics_UploadProblem,  0, HOOK_PRIORITY_NORMAL);

  INF_SAFE_INT_KEY (checker_upload_interval, "Checker/UploadInterval", INFORMATICS_CHECKER_UPLOAD_INTERVAL);
  RESET_LEZ (checker_upload_interval, INFORMATICS_CHECKER_UPLOAD_INTERVAL);
  scheduler_add (Informatics_UploadChecker, 0, checker_upload_interval*USEC_COUNT);
  return 0;
}

static int
OnUnload                           (plugin_t *__plugin)
{
  hook_unregister (CORE_ACTIVATE,    activate,                 HOOK_PRIORITY_NORMAL);
  hook_unregister (CORE_DEACTIVATE,  deactivate,               HOOK_PRIORITY_NORMAL);
  hook_unregister (CORE_STOPTESTING, Informatics_stop_testing, HOOK_PRIORITY_NORMAL);

  hook_unregister (CORE_STOPTESTING, Informatics_StopCheckerUploading, HOOK_PRIORITY_NORMAL);
  hook_unregister (CORE_STOPTESTING, Informatics_StopProblemUploading, HOOK_PRIORITY_NORMAL);

  hook_unregister (CORE_UPLOADPROBLEMS, Informatics_UploadProblem,  HOOK_PRIORITY_NORMAL);

  scheduler_remove (Informatics_UploadChecker);
	return 0;
}

////////////////////////
// Plugin registration info

static plugin_info_t Info={
  INFORMATICS_MAJOR_VERSION, // Major version
  INFORMATICS_MINOR_VERSION, // Minor version

  0,        // Onload handler
  OnUnload  // OnUnload handler
};

PLUGIN_INIT  (INFORMATICS_LIBNAME, Init, Info);
