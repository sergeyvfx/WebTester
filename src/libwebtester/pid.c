/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "pid.h"
#include "fs.h"
#include "util.h"
#include "assarr.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>

static int openedCount = 0;

static int
pid_get_fileno (char *__fn)
{
  return open (__fn, O_CREAT);
}

/**
 * Create pid file
 *
 * @param __fn - name of file
 * @return zero on success, non-zero otherwise
 */
int
create_pid_file (char *__fn)
{
  char dir[65536];
  FILE *stream;
  dirname (__fn, dir);
  fmkdir (dir, 0775);
  stream = fopen (__fn, "w");
  if (stream)
    {
      int result = -1;
      int fileNo = pid_get_fileno (__fn);
      if (!flock (fileNo, LOCK_EX | LOCK_NB))
        {
          openedCount++;
          fprintf (stream, "%d", getpid ());
          result = 0;
        }
      fclose (stream);
      return result;
    }
  return -1;
}

/**
 * Delete pid file
 *
 * @param __fn - name of file
 * @return zero on success, non-zero otherwise
 */
int
delete_pid_file (char *__fn)
{
  int fileNo = pid_get_fileno (__fn);
  flock (fileNo, LOCK_UN);
  close (fileNo);
  unlink (__fn);
  openedCount--;
  return 0;
}
