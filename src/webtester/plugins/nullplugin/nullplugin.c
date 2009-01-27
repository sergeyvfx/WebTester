/**
 * WebTester Server - server of on-line testing system
 *
 * Sample plugin for Webtester
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <libwebtester/plugin-defs.h>
#include <libwebtester/core.h>
#include <libwebtester/hook.h>

#include "nullplugin.h"

/**
 * Handler of plugin activation
 *
 * @return zero on success, non-zero otherwise
 */
static int
activate (void *__unused, void *__call_unused)
{
  return 0;
}

/**
 * Handler of plugin deactivation
 *
 * @return zero on success, non-zero otherwise
 */
static int
deactivate (void *__unused, void *__call_unused)
{
  return 0;
}

/**
 * Plugin initialization
 *
 * @param __plugin - plugin descriptor
 * @return zero on success, non-zero otherwise
 */
static int
Init (plugin_t *__plugin)
{
  hook_register (CORE_ACTIVATE,   activate,   0, HOOK_PRIORITY_NORMAL);
  hook_register (CORE_DEACTIVATE, deactivate, 0, HOOK_PRIORITY_NORMAL);

  return 0;
}

/**
 * Handler of plugin unloading
 *
 * @param __plugin - plugin descriptor
 * @return zero on success, non-zero otherwise
 */
static int
OnUnload (plugin_t *__plugin)
{
  hook_unregister (CORE_ACTIVATE, activate, HOOK_PRIORITY_NORMAL);
  hook_unregister (CORE_DEACTIVATE, deactivate, HOOK_PRIORITY_NORMAL);

  return 0;
}

/****
 * PLugin information struct
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
