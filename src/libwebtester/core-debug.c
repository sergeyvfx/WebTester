/*
 *
 * =============================================================================
 *  core-debug.c
 * =============================================================================
 *
 *  Written (by Nazgul) under GPL
 *
*/

#include "core.h"
#include "smartinclude.h"

#include <stdio.h>

static int debugMode=0;

void
core_enter_debug_mode              (void)
{
  debugMode=1;
}

int
core_is_debug_mode                 (void)
{
  return debugMode;
}
