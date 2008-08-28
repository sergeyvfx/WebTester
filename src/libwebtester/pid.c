/*
 *
 * ================================================================================
 *  pid.c
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
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

static int openedCount=0;

int
pid_get_fileno                     (char *__fn)
{
  return open (__fn, O_CREAT);
}

int
create_pid_file                    (char *__fn)
{
  char dir[65536];
  FILE *stream;
  dirname (__fn, dir);
  fmkdir (dir, 0775);
  stream=fopen (__fn, "w");
  if (stream)
    {
      int result=-1;
      int fileNo=pid_get_fileno (__fn);
      if (!flock (fileNo, LOCK_EX|LOCK_NB)) {
        openedCount++;
        fprintf (stream, "%d", getpid ());
        result=0;
      }
      fclose (stream);
      return result;
    }
  return -1;
}

int
delete_pid_file                    (char *__fn)
{
  int fileNo=pid_get_fileno (__fn);
  flock (fileNo, LOCK_UN);
  close (fileNo);
  unlink (__fn);
  openedCount--;
  return 0;
}
