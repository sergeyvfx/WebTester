/*
 * ================================================================================
 *  testlib.h - part of the TestLib
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_testlib_h_
#define _wt_testlib_h_

#include <stdio.h>
#include <string.h>

#define _OK 0x0000
#define _WA 0x0001
#define _PE 0x0002
#define _CR 0x0004

////////////////////////////////////////
// Some useful defenitions

#define Quit(__errno, __text) \
  testlib_quit (__errno, __text);

////////////////////////////////////////
// Parsing and readin

#ifdef __cplusplus
extern "C" {
#endif

int
testlib_read_integer               (FILE *__stream);

long
testlib_read_longint               (FILE *__stream);

double
testlib_read_float                 (FILE *__stream);

int
testlib_read_char                  (FILE *__stream);

int
testlib_cur_char                   (FILE *__stream);

void
testlib_read_string                (FILE *__stream, char *__buf, int __maxlen);

int
testlib_seekeof                    (FILE *__stream);

int
testlib_eof                        (FILE *__stream);

int
testlib_eoln                       (FILE *__stream);

int
testlib_seekeoln                   (FILE *__stream);

void
testlib_next_line                  (FILE *__stream);

void
testlib_skip                       (FILE *__stream, char *__charset);

////
//
  
void
testlib_set_output_stream          (FILE *__stream);

void
testlib_silent                     (int __val);

////////////////////////////////////////
// Deep-side stuff

void
testlib_quit                       (int __errno, char *__desc);

#ifdef __cplusplus
}
#endif

#ifndef TESTLIB_SO
#  ifndef _wt_testlibpp_h_

FILE *inf=0, *ouf=0, *ans=0;

void
Check                              (void);

int
main                               (int __argc, char **__argv)
{
  if (__argc<4)
    {
      char usage[4096];
      sprintf (usage, "Usage: %s <input file> <output file> <answer file>", __argv[0]);
      Quit (-1, usage);
    }

  if (__argc>4 && !strcmp (__argv[4], "-s"))
    testlib_silent (1);

  inf=fopen (__argv[1], "r");
  ouf=fopen (__argv[2], "r");
  ans=fopen (__argv[3], "r");

  if (!ouf)
    Quit (_PE, "File not found");

  testlib_set_output_stream (ouf);

  Check ();

  Quit (_OK, "");

  return 0;
}
#  endif
#endif

#endif
