/**
 * WebTester Server - server of on-line testing system
 *
 * Sample module for WebTester
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <webtester/autoinc.h>
#include <webtester/task.h>

#include <libwebtester/plugin-defs.h>
#include <libwebtester/core.h>
#include <libwebtester/hook.h>

/**
 * Handler of module activation
 *
 * @return zero on success, non-zero otherwise
 */
static int
activate (void *__unused, void *__call_unused)
{
  return 0;
}

/**
 * Handler of module deactivation
 *
 * @return zero on success, non-zero otherwise
 */
static int
deactivate (void *__unused, void *__call_unused)
{
  return 0;
}

/**
 * Send task for testing
 *
 * @param __task - task to be tested
 * @param __error - buffer for error description
 * @return TRUE isf task is sent, FALSE otherwise
 */
BOOL
StartForTesting (wt_task_t *__task, char *__error)
{
  TASK_SET_STATUS (*__task, TS_FINISHED);

  return TRUE;
}

/**
 * Module initialization
 *
 * @param __plugin - plugin descriptor
 * @return zero on success, non-zero otherwise
 */
static int
Init (plugin_t *__plugin)
{
  hook_register (CORE_ACTIVATE, activate, 0, HOOK_PRIORITY_NORMAL);
  hook_register (CORE_DEACTIVATE, deactivate, 0, HOOK_PRIORITY_NORMAL);
  return 0;
}

/**
 * Handler of module unloading
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
 * Module registration info
 */

static plugin_info_t Info = {
  1, /* Major version */
  1, /* Minor version */

  0,        /* Onload handler */
  OnUnload, /* OnUnload handler */

  0,  /* No activation callback */
  0   /* No deactivation callback */
};

PLUGIN_INIT ("nullmodule", Init, Info);
