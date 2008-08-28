/*
 *
 * ================================================================================
 *  webinterface.c - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <libwebtester/plugin-defs.h>
#include <libwebtester/core.h>
#include <libwebtester/hook.h>

#include <webtester/autoinc.h>

#include "webinterface.h"

static int
activate                           (void *__unused)
{
  if (webiface_transport_init ())
    {
      core_print (MSG_ERROR, "    **** Error creating HTTP WebIface session.\n");
    }
  core_print (MSG_INFO, "    **** HTTP WebIface access plugin activated,\n");
  return 0;
}

static int
deactivate                         (void *__deactivate)
{
  webiface_transport_done ();
  core_print (MSG_INFO, "    **** HTTP WebIface access plugin deactivated,\n");
  return 0;
}

static int
Init                               (plugin_t *__plugin)
{
  hook_register (CORE_ACTIVATE,   activate,   0, HOOK_PRIORITY_HIGHT);
  hook_register (CORE_DEACTIVATE, deactivate, 0, HOOK_PRIORITY_NORMAL);

  return 0;
}

static int
OnUnload                           (plugin_t *__plugin)
{
  hook_unregister (CORE_ACTIVATE,   activate,   HOOK_PRIORITY_HIGHT);
  hook_unregister (CORE_DEACTIVATE, deactivate, HOOK_PRIORITY_NORMAL);
	return 0;
}

////////////////////////
//

static plugin_info_t Info={
  WEBIFACE_MAJOR_VERSION,
  WEBIFACE_MINOR_VERSION,

  0,
  OnUnload
};

PLUGIN_INIT  (WEBIFACE_LIBNAME, Init, Info);
