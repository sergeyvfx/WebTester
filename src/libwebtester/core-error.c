/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "core.h"
#include "macrodef.h"

#include <stdarg.h>
#include <string.h>

static char core_error[4096];

/**
 * Set last CORE error
 *
 * @param __text - description of error
 */
void
core_set_last_error (char *__text, ...)
{
  static char buf[4096];
  PACK_ARGS (__text, buf, 4096);
  strcpy (core_error, buf);
}

/**
 * Get description of las error
 *
 * @return description of las error
 */
char*
core_get_last_error (void)
{
  return core_error;
}
