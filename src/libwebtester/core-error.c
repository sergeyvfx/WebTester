/*
 *
 * ================================================================================
 *  core-error.c
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "core.h"
#include "macrodef.h"

#include <stdarg.h>
#include <string.h>

static char core_error[4096];

void
core_set_last_error                (char *__text, ...)
{
  static char buf[4096];
  PACK_ARGS (__text, buf, 4096);
  strcpy (core_error, buf);
}

char*
core_get_last_error                (void)
{
  return core_error;
}
