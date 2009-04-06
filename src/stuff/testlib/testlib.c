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

#define TESTLIB_SO

#include "testlib.h"
#include "util.h"
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#define PRINT(__text,__args...) \
    fprintf (stderr, __text, ##__args)

static FILE *out_stream = 0;
static int silent = 0;
static int colorized = 1;

/**
 * Go back in strem
 *
 * @param __stream - strem to go back in
 */
static void
goback (FILE *__stream)
{
  if (!feof (__stream))
    {
      fseek (__stream, -1, SEEK_CUR);
    }
}

/****
 * Parsing and readin
 */

/**
 * Read signed number from stream
 *
 * @param __stream - stream to read from
 * @param __max - max absolute value of number
 * @return read number
 */
static long
read_number (FILE *__stream, long long __max)
{
  int sign = 1, readed = 0;
  int ch;
  long long val = 0;

  if (!__stream)
    {
      Quit (_CR, "Specified file not opened");
    }

  /* Scip forwarding spaces */
  ch = fgetc (__stream);
  while (is_space (ch) && ch != EOF)
    {
      ch = fgetc (__stream);
    }

  if (ch == EOF)
    {
      Quit (_PE, "Unexpected end of file");
    }

  /* Get sign of the number */
#ifdef ALLOW_POSITIVE_SIGN
  if (ch == '-' || ch == '+')
#else
  if (ch == '-')
#endif
    {
      if (ch == '-')
        {
          sign = -1;
        }
      ch = fgetc (__stream);
    }

  /* Collect number */
  while (ch >= '0' && ch <= '9')
    {
      readed = 1;
      val = val * 10 + ch - '0';
#ifdef ENABLE_RANGE_CHECK
      if (sign * val<-__max - 1 || sign * val > __max)
        {
          Quit (_PE, "Invalid integer value (Range Check error).");
        }
#endif
      ch = fgetc (__stream);
    }

  if ((!is_space (ch) && ch != EOF) || !readed)
    {
      Quit (_PE, "Invalid integer value.")
    }

  goback (__stream);

  return val * sign;
}

/**
 * Read integer value from stream
 *
 * @param __stream - stream to read from
 * @return read integer value
 */
int
testlib_read_integer (FILE *__stream)
{
  return read_number (__stream, MAXINT);
}

/**
 * Read long integer value from stream
 *
 * @param __stream - stream to read from
 * @return read long integer value
 */
long
testlib_read_longint (FILE *__stream)
{
  return read_number (__stream, MAXLONGINT);
}

/**
 * Read double value from stream
 *
 * @param __stream - stream to read from
 * @return read double value
 */
double
testlib_read_float (FILE *__stream)
{
  int sign = 1, readed = 0, decimal, total;
  int ch;
  double val = 0;

  if (!__stream)
    {
      Quit (_CR, "Specified file not opened");
    }

  /* Scip forwarding spaces */
  ch = fgetc (__stream);
  while (is_space (ch) && ch != EOF)
    {
      ch = fgetc (__stream);
    }

  /* Get sign of the number */
#ifdef ALLOW_FLOAT_POSITIVE_SIGN
  if (ch == '-' || ch == '+')
#else
  if (ch == '-')
#endif
    {
      if (ch == '-')
        sign = -1;
      ch = fgetc (__stream);
    }

  /* Collect number */

  decimal = -1;
  total = 0;

  while ((ch >= '0' && ch <= '9') || ch == '.' || ch == ',')
    {
      if (ch == '.' || ch == ',')
        {
          /* Decimal point is already caught */
          if (decimal > 0)
            {
              Quit (_PE, "Invalid floating-point value.");
            }
          decimal = total;
          readed = 0;
          ch = fgetc (__stream);
          continue;
        }

      readed = 1;
      val = val * 10 + ch - '0';
      ch = fgetc (__stream);

      total++;
    }

  if ((!is_space (ch) && ch != EOF) || !readed)
    {
      Quit (_PE, "Invalid floating-point value.");
    }

  goback (__stream);

  if (decimal < 0)
    {
      return val * sign;
    }

  while (total > decimal)
    {
      val /= 10;
      total--;
    }

  return val*sign;
}

/**
 * Read string from stream
 *
 * @param __stream - stream to read from
 * @param __buf - buffer to store string in
 * @param __maxlen - max length of buffer
 */
void
testlib_read_string (FILE *__stream, char *__buf, int __maxlen)
{
  int len = 0;
  int ch;

  if (!__stream)
    {
      Quit (_CR, "Specified file not opened");
    }

  ch = fgetc (__stream);

  if (ch == EOF)
    {
      Quit (_PE, "Unexpected end of file");
    }

  /* Read string */
  while (ch != '\n' && ch != '\r' && ch != EOF && len < __maxlen)
    {
      __buf[len++] = ch;
      ch = fgetc (__stream);
    }

  if (ch == '\r')
    {
      if (fgetc (__stream) != '\n')
        {
          goback (__stream);
        }
    }
  else
    {
      if (ch >= 0)
        {
          goback (__stream);
        }
    }

  __buf[len] = 0;
}

/**
 * Read char from stream and go to next
 *
 * @param __stream - stream to read from
 * @return read char code
 */
int
testlib_read_char (FILE *__stream)
{
  int ch;

  if (!__stream)
    {
      Quit (_CR, "Specified file not opened");
    }

  ch = fgetc (__stream);
  if (ch == EOF)
    {
      Quit (_PE, "No chars to read.");
    }

  return ch;
}

/**
 * Return current character in stream
 *
 * @param __stream - stream to read from
 * @return current character code
 */
int
testlib_cur_char (FILE *__stream)
{
  int ch;

  if (!__stream)
    {
      Quit (_CR, "Specified file not opened");
    }

  ch = fgetc (__stream);
  goback (__stream);

  if (ch == EOF)
    {
      Quit (_PE, "No chars to read.");
    }

  return ch;
}

/**
 * Test end of stream
 *
 * @param __stream - stream to test
 * @return non-zero if stream is over, zero otherwise
 */
int
testlib_eof (FILE *__stream)
{
  if (!__stream)
    {
      Quit (_CR, "Specified file not opened");
    }

  int ch = fgetc (__stream);
  int res = ch == EOF;
  goback (__stream);
  return res;
}

/**
 * Test end of line in stream
 *
 * @param __stream - stream to test
 * @return non-zero if line is over, zero otherwise
 */
int
testlib_eoln (FILE *__stream)
{
  if (!__stream)
    {
      Quit (_CR, "Specified file not opened");
    }

  int ch = fgetc (__stream);
  int res = ch == '\n' || ch == '\r' || ch == EOF;
  goback (__stream);
  return res;
}

/**
 * Seek end of file in stream
 *
 * @param __stream - stream to test
 * @return non-zero if stream is near to be over, zero otherwise
 */
int
testlib_seekeof (FILE *__stream)
{
  int ch, res;
  size_t pos;

  if (!__stream)
    {
      Quit (_CR, "Specified file not opened");
    }

  if (feof (__stream))
    {
      return 1;
    }

  pos = fposget (__stream);

  ch = fgetc (__stream);
  while (is_space (ch) && ch != EOF)
    {
      ch = fgetc (__stream);
    }

  res = ch == EOF;

  fposset (__stream, pos);

  return res;
}

/**
 * Seek end of line in stream
 *
 * @param __stream - stream to test
 * @return non-zero if stream is line to be over, zero otherwise
 */
int
testlib_seekeoln (FILE *__stream)
{
  int ch, res;
  size_t pos;

  if (!__stream)
    {
      Quit (_CR, "Specified file not opened");
    }

  if (feof (__stream))
    {
      return 1;
    }

  pos = fposget (__stream);

  ch = fgetc (__stream);
  while (is_space (ch) && (ch != '\n' && ch != '\r' && ch != EOF))
    {
      ch = fgetc (__stream);
    }

  res = ch == EOF || ch == '\r' || ch == '\n';

  fposset (__stream, pos);

  return res;
}

/**
 * Skip characters and move to next line
 *
 * @param __stream - stream to skip in
 */
void
testlib_next_line (FILE *__stream)
{
  int ch;

  if (!__stream)
    {
      Quit (_CR, "Specified file not opened");
    }

  ch = fgetc (__stream);
  while (ch != '\r' && ch != '\n')
    {
      if (ch == EOF) Quit (_PE, "Unexpected end of file");
      ch = fgetc (__stream);
    }

  if (ch == '\n')
    {
      ch = fgetc (__stream);
      if (ch == EOF) Quit (_PE, "Unexpected end of file");
      if (ch != '\r') goback (__stream);
    }
  else
    {
      if (ch == EOF) Quit (_PE, "Unexpected end of file");
      goback (__stream);
    }
}

/**
 * Skip characterd from charset. Do not generate errors
 *
 * @param __stream - stream to skip in
 * @param __charset - set of chars to skip
 */
void
testlib_skip (FILE *__stream, const char *__charset)
{
  int ch, i, n;
  int skip_chars[255];

  if (!__stream)
    Quit (_CR, "Specified file not opened");

  memset (skip_chars, 0, sizeof (skip_chars));

  for (i = 0, n = strlen (__charset); i < n; i++)
    {
      skip_chars[(int) __charset[i]] = 1;
    }

  ch = fgetc (__stream);
  while (ch != EOF && skip_chars[ch])
    {
      ch = fgetc (__stream);
    }

  if (ch != EOF)
    {
      goback (__stream);
    }
}

/**
 * Set output stream
 *
 * @param __stream - stream to use
 */
void
testlib_set_output_stream (FILE *__stream)
{
  out_stream = __stream;
}

/**
 * Print quit message prefix
 *
 * @param __errno - error number
 */
static void
quit_message_prefix (int __errno)
{
  if (__errno != _OK)
    {
      if (colorized)
        {
          PRINT ("\33[31;1m");
        }

      if (__errno == _WA)
        {
          PRINT ("WA");
        }
      if (__errno == _PE)
        {
          PRINT ("PE");
        }
      if (__errno == _CR)
        {
          PRINT ("CR");
        }

      if (colorized)
        {
          PRINT ("\33[0m");
        }
    }
  else
    {
      if (colorized)
        {
          PRINT ("\33[32;1mOK\33[0m");
        }
      else
        {
          PRINT ("OK");
        }
    }
}

/**
 * Print quit message
 *
 * @param __errno - error number
 * @param __desc - error descriptor
 */
static void
quit_message (int __errno, const char *__desc)
{
  quit_message_prefix (__errno);
  if (strcmp (__desc, ""))
    {
      PRINT ("  %s", __desc);
    }
  PRINT ("\n");

  if (!silent && __errno != _OK)
    {
      /* PRINT ("\33[10;50;11;1000]\7"); */
      usleep (200000);
    }
}

/**
 * Quit from testlib
 *
 * @param __errno - error number
 * @param __desc - description of error
 */
void
testlib_quit (int __errno, const char *__desc, ...)
{
  char desc[65536];
  va_list ap;

  va_start (ap, __desc);
  vsnprintf (desc, sizeof (desc), __desc, ap);
  va_end (ap);

  if (__errno == _OK)
    {
      if (!out_stream)
        {
          quit_message (_CR, "Output file not assigned.");
          exit (_CR);
        }

      if (testlib_seekeof (out_stream))
        {
          quit_message (__errno, desc);
          exit (__errno);
        }

      quit_message (_WA, "Extra information in tail of the output file.");
      exit (_WA);
    }

  quit_message (__errno, desc);
  exit (__errno);
}

/**
 * Set testlib silence
 *
 * @param __val - silence flag
 */
void
testlib_silent (int __val)
{
  silent = __val;
}

/**
 * Set testlib colorized
 *
 * @param __val - colorized flag
 */
void
testlib_colorized (int __val)
{
  colorized = __val;
}
