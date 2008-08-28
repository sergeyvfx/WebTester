/*
 *
 * ================================================================================
 *  log.c - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "log.h"
#include "macrodef.h"
#include "fs.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <malloc.h>

#include <glib.h>

#include "mutex.h"

static char filename[4096];
static FILE *stream=0;
static int put_timestamp=1;

static mutex_t mutex=0;

static void
logfile_open                       (void)
{
  int print_banner=0;
  char dir[4096];

  dirname (filename, dir);

  fmkdir (dir, 00775);

  if (stream)
    fclose (stream);

  if (!fexists (filename))
    print_banner=1;

  stream=fopen (filename, "a");

  if (print_banner)
    {
      fprintf (stream, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
      fprintf (stream, "%s\n", core_get_version_string ());
      fprintf (stream, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");
    } else
      fprintf (stream, "\n\n");

  chmod (filename, 0640);
}

static void
logfile_close                      (void)
{
  if (stream)
    {
      fclose (stream);
      stream=0;
    }
}

static gpointer
logfile_pack_entry                 (gpointer __fname)
{
  char *fn=__fname;
  pack_file (fn, LOG_PACKER);
  free (fn);

  g_thread_exit (0);
  return 0;
}

static void
logfile_pack                       (void)
{
  char *fname=malloc (4096);
  GThread *thread;
  fdup (filename, fname, LOG_PACKED_EXT, MAX_LOG_FILES);

  thread=g_thread_create (logfile_pack_entry, fname, FALSE, 0);
  
  unlink (filename);
}

static void
check_size                         (void)
{
  if (fsize (stream)>MAX_LOG_SIZE)
    {
      printf ("Packing LOG file...\n");
      logfile_close ();
      logfile_pack ();
      logfile_open ();
    }
}

////////
//

int
log_init                           (const char *__fn)
{
  mutex=mutex_create ();

  strcpy (filename, __fn);

  logfile_open ();
  
  if (!stream)
    return -1;
  return 0;
}

void
log_done                           (void)
{
  logfile_close ();

  mutex_free (mutex);
}

void
log_printf                         (char *__text, ...)
{
  char print_buf[4096];
  char timestamp[128];

  mutex_lock (mutex);

  if (!stream)
    return;

  PACK_ARGS (__text, print_buf, 4096);

  get_datetime_strf (timestamp, 128, "%F %T");

  if (put_timestamp)
    fprintf (stream, "[%s] %s", timestamp, print_buf); else
    fputs (print_buf, stream);

  put_timestamp=1;
  if (print_buf[strlen (print_buf)-1]!='\n')
    put_timestamp=0;

  fflush (stream);

  check_size ();

  mutex_unlock (mutex);
}
