/**
 * WebTester Server - server of on-line testing system
 *
 * Masked random password generator
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

int
main (int __argc, char **__argv)
{
  int i, n, t, v;
  char pass[1000];
  struct timeval tv;
  char *data[] ={
    "qQwWrRtTpPsSdDfFgGjJhHklLZzXxcCvVbBnNmM",
    "eEyYuUiIoOaA"
  };

  gettimeofday (&tv, 0);
  srand (time (0) + tv.tv_usec);

  if (__argc != 2)
    {
      printf ("Usage: %s mask\n", __argv[0]);
      return -1;
    }

  n = strlen (__argv[1]);
  i = 0;
  for (i = 0; i < n; i++)
    {
      t = __argv[1][i] - '0';
      v = rand () % strlen (data[t]);
      pass[i] = data[t][v];
    }
  pass[i] = 0;
  printf ("%s", pass);

  return 0;
}
