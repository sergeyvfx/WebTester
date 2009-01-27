/**
 * WebTester Server - server of on-line testing system
 *
 * Processes' based stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "core.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

/**
 * Kill process
 *
 * @param __pid - ID of process to be killed
 * @param __signal - signal with which process will be killed
 */
void
core_kill_process (int __pid, int __signal)
{
  kill (__pid, __signal);
}
