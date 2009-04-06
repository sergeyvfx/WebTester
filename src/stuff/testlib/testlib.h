/**
 * WebTester Server - server of on-line testing system
 *
 * Main implementation file of TestLib
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_TESTLIB_H_
#define _WT_TESTLIB_H_

#include <stdio.h>
#include <string.h>

#define _OK 0x0000
#define _WA 0x0001
#define _PE 0x0002
#define _CR 0x0004

/****
 * Some useful defenitions
 */

#define Quit(__errno, __text,__args...) \
  testlib_quit (__errno, __text, ##__args);

#ifdef __cplusplus
extern "C"
{
#endif

  /****
   * Parsing and readin
   */

  /* Read integer value from stream */
  int
  testlib_read_integer (FILE *__stream);

  /* Read long integer value from stream */
  long
  testlib_read_longint (FILE *__stream);

  /* Read double value from stream */
  double
  testlib_read_float (FILE *__stream);

  /* Read char from stream and go to next */
  int
  testlib_read_char (FILE *__stream);

  /* Return current character in stream */
  int
  testlib_cur_char (FILE *__stream);

  /* Read string from stream */
  void
  testlib_read_string (FILE *__stream, char *__buf, int __maxlen);

  /* Seek end of file in stream */
  int
  testlib_seekeof (FILE *__stream);

  /* Test end of stream */
  int
  testlib_eof (FILE *__stream);

  /* Test end of line in stream */
  int
  testlib_eoln (FILE *__stream);

  /* Seek end of line in stream */
  int
  testlib_seekeoln (FILE *__stream);

  /* Skip characters and move to next line */
  void
  testlib_next_line (FILE *__stream);

  /* Skip characterd from charset. Do not generate errors */
  void
  testlib_skip (FILE *__stream, const char *__charset);

  /* Set output stream */
  void
  testlib_set_output_stream (FILE *__stream);

  /* Set testlib silence */
  void
  testlib_silent (int __val);

  /* Set testlib colorized */
  void
  testlib_colorized (int __val);

  /* Quit from testlib */
  void
  testlib_quit (int __errno, const char *__desc, ...);

#ifdef __cplusplus
}
#endif

#ifndef TESTLIB_SO
#  ifndef _WT_TESTLIB_H_

FILE *inf = 0, *ouf = 0, *ans = 0;

void
Check (void);

int
main (int __argc, char **__argv)
{
  if (__argc < 4)
    {
      char usage[4096];
      sprintf (usage, "Usage: %s <input file> <output file> "
                      "<answer file> [-s] [-nc]",
               __argv[0]);
      Quit (-1, usage);
    }

  if (__argc > 4)
    {
      int i;

      for (i = 4; i < __argc; ++i)
        {
          if (!strcmp (__argv[i], "-s"))
            {
              testlib_silent (1);
            }
          else if (!strcmp (__argv[i], "-nc"))
            {
              testlib_colorized (0);
            }
        }
    }

  inf = fopen (__argv[1], "r");
  ouf = fopen (__argv[2], "r");
  ans = fopen (__argv[3], "r");

  if (!ouf)
    {
      Quit (_PE, "File not found");
    }

  testlib_set_output_stream (ouf);

  Check ();

  Quit (_OK, "");

  return 0;
}

#  endif
#endif

#endif
