/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "core.h"

#include <stdio.h>

static int debugMode = 0;

/**
 * Enter CORE to DEBUG mode
 */
void
core_enter_debug_mode (void)
{
  debugMode = 1;
}

/**
 * Check is CORE in debuf mode
 *
 * @return is CORE in debuf mode
 */
int
core_is_debug_mode (void)
{
  return debugMode;
}
