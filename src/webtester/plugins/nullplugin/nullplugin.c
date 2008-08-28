/*
 *
 * ================================================================================
 *  nullplugin.c - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <libwebtester/plugin-defs.h>
#include <libwebtester/core.h>
#include <libwebtester/hook.h>

#include <webtester/autoinc.h>

#include "nullplugin.h"

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
  OnUnload,

  0,
  0
};

PLUGIN_INIT  (RUNDLL_LIBNAME, Init, Info);
