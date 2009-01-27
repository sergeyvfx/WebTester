/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <libwebtester/plugin-defs.h>
#include <libwebtester/core.h>
#include <libwebtester/hook.h>

#include "webinterface.h"

/**
 * Plugin activation callback
 *
 * @return zero on success, non-zero otherwise
 */
static int
activate (void *__unused, void *__call_unused)
{
  if (webiface_transport_init ())
    {
      core_print (MSG_ERROR, "    **** Error creating "
                             "HTTP WebIface session.\n");
    }

  core_print (MSG_INFO, "    **** HTTP WebIface access plugin activated,\n");

  return 0;
}

/**
 * Plugin deactivation callback
 *
 * @return zero on success, non-zero otherwise
 */
static int
deactivate (void *__deactivate, void *__call_unused)
{
  webiface_transport_done ();
  core_print (MSG_INFO, "    **** HTTP WebIface access plugin deactivated,\n");
  return 0;
}

/**
 * Initialize plugin
 *
 * @param __plugin - plugin descriptor
 * @return zero on success, non-zero otherwise
 */
static int
Init (plugin_t *__plugin)
{
  hook_register (CORE_ACTIVATE, activate, 0, HOOK_PRIORITY_HIGHT);
  hook_register (CORE_DEACTIVATE, deactivate, 0, HOOK_PRIORITY_NORMAL);

  return 0;
}

/**
 * Unload plugin
 *
 * @param __plugin - plugin descriptor
 * @return zero on success, non-zero otherwise
 */
static int
OnUnload (plugin_t *__plugin)
{
  hook_unregister (CORE_ACTIVATE, activate, HOOK_PRIORITY_HIGHT);
  hook_unregister (CORE_DEACTIVATE, deactivate, HOOK_PRIORITY_NORMAL);
  return 0;
}

/****
 * Plugin info struct
 */
static plugin_info_t Info = {
  WEBIFACE_MAJOR_VERSION,
  WEBIFACE_MINOR_VERSION,

  0,
  OnUnload,
  0,
  0
};

PLUGIN_INIT (WEBIFACE_LIBNAME, Init, Info);
