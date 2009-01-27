/**
 * WebTester Server - server of on-line testing system
 *
 * Unique ID generator
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <uid.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>

#include "md5.h"

/**
 * Generate unique ID
 *
 * @param __out - generated id
 * @return the same as __out
 */
char*
uid_gen (char *__out)
{
  char salt[9] = {0}, pass[128] = {0}, dummy[1024];
  int i;
  struct timeval tv;

  if (!__out) return 0;

  gettimeofday (&tv, 0);
  srand (time (0) + tv.tv_usec);

  for (i = 0; i < 8; i++)
    {
      salt[i] = rand () % 220 + 32;
    }

  sprintf (pass, "%ld-%s", (long) time (0), salt);

  md5_crypt (pass, salt, dummy);

  strcpy (__out, dummy + 8);

  return __out;
}
