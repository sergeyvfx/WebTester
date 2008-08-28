/*
 *
 * =============================================================================
 *  core-proc.c
 * =============================================================================
 *
 *  Processes' based stuff
 *
 *  Written (by Nazgul) under GPL
 *
*/

#include "core.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

void
core_kill_process                  (int __pid, int __signal)
{
  if (kill (__pid, __signal))
    perror ("kill");
}
