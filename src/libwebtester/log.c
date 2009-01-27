/**
 * WebTester Server - server of on-line testing system
 *
 * Hooks' stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
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

#include "thread.h"

static char filename[4096];
static FILE *stream = 0;
static int put_timestamp = 1;

/**
 * Open LOG file
 */
static void
logfile_open (void)
{
  int print_banner = 0;
  char dir[4096];

  dirname (filename, dir);

  fmkdir (dir, 00775);

  if (stream)
    {
      fclose (stream);
    }

  if (!fexists (filename))
    {
      print_banner = 1;
    }

  stream = fopen (filename, "a");

  if (!stream)
    {
      return;
    }

  if (print_banner)
    {
      fprintf (stream, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
                       "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
      fprintf (stream, "%s\n", core_get_version_string ());
      fprintf (stream, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
                       "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");
    }
  else
    {
      fprintf (stream, "\n\n");
    }

  chmod (filename, 0640);
}

/**
 * Close LOG file
 */
static void
logfile_close (void)
{
  if (stream)
    {
      fclose (stream);
      stream = 0;
    }
}

/**
 * Entry point for LOG file packing
 *
 * @param __fname - name of file to pack
 */
static gpointer
logfile_pack_entry (gpointer __fname)
{
  char *fn = __fname;

  pack_file (fn, LOG_PACKER);
  free (fn);

  g_thread_exit (0);
  return 0;
}

/**
 * Pack LOG file
 */
static void
logfile_pack (void)
{
  char *fname = malloc (4096);
  GThread *thread;
  fdup (filename, fname, LOG_PACKED_EXT, MAX_LOG_FILES);

  thread = g_thread_create (logfile_pack_entry, fname, FALSE, 0);

  unlink (filename);
}

/**
 * Check size of LOG file
 */
static void
check_size (void)
{
  if (fsize (stream) > MAX_LOG_SIZE)
    {
      printf ("Packing LOG file...\n");
      logfile_close ();
      logfile_pack ();
      logfile_open ();
    }
}

/********
 * User's backend
 */

/**
 * Initialize LOG stuff
 *
 * @param __fn - name of LOG file
 * @return zero on success, non-zero otherwise
 */
int
log_init (const char *__fn)
{
  strcpy (filename, __fn);

  logfile_open ();

  if (!stream)
    {
      return -1;
    }

  return 0;
}

/**
 * Uninitialize LOG stuff
 */
void
log_done (void)
{
  logfile_close ();
}

/**
 * Print text to LOG file
 *
 * @oaram __text - teft t oprint
 */
void
log_printf (char *__text, ...)
{
  char print_buf[4096];
  char timestamp[128];

  CS_Begin

  if (!stream)
    {
      CS_End
      return;
    }

  PACK_ARGS (__text, print_buf, 4096);

  get_datetime_strf (timestamp, 128, "%F %T");

  if (put_timestamp)
    {
      fprintf (stream, "[%s] %s", timestamp, print_buf);
    }
  else
    {
      fputs (print_buf, stream);
    }

  put_timestamp = 1;

  if (print_buf[strlen (print_buf) - 1] != '\n')
    {
      put_timestamp = 0;
    }

  fflush (stream);

  check_size ();

  CS_End
}
