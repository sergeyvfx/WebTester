/*
 * ================================================================================
 *  testlib.c - part of the TestLib
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#define TESTLIB_SO

#include "testlib.h"
#include "util.h"
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PRINT(__text,__args...) \
    fprintf (stderr, __text, ##__args)

static FILE *out_stream=0;
static int silent=0;

////////
//

static void
goback                             (FILE *__stream)
{
  if (!feof (__stream))
    fseek (__stream, -1, SEEK_CUR);
}

////////////////////////////////////////
// Parsing and readin

static long   // Read signed number from stream
read_number                        (FILE *__stream, long long __max)
{
  int sign=1, readed=0;
  int ch;
  long long val=0;

  if (!__stream)
    Quit (_CR, "Specified file not opened");

  // Scip forwarding spaces
  ch=fgetc (__stream);
  while (is_space (ch) && ch!=EOF)
    ch=fgetc (__stream);

  if (ch==EOF)
    Quit (_PE, "Unexpected end of file");

  // Get sign of the number
#ifdef ALLOW_POSITIVE_SIGN
  if (ch=='-' || ch=='+')
#else
  if (ch=='-')
#endif
    {
      if (ch=='-')
        sign=-1;
      ch=fgetc (__stream);
    }

  // Collect number
  while (ch>='0' && ch<='9')
    {
      readed=1;
      val=val*10+ch-'0';
#ifdef ENABLE_RANGE_CHECK
      if (sign*val<-__max-1 || sign*val>__max)
        Quit (_PE, "Invalid integer value (Range Check error).");
#endif
      ch=fgetc (__stream);
    }

  if ((!is_space (ch) && ch!=EOF) || !readed)
    Quit (_PE, "Invalid integer value.")

  goback (__stream);

  return val*sign;
}

int    // Read integer value from stream
testlib_read_integer               (FILE *__stream)
{
  return read_number (__stream, MAXINT);
}

long   // Read longint value from stream
testlib_read_longint               (FILE *__stream)
{
  return read_number (__stream, MAXLONGINT);
}

double  // Read double value from stream
testlib_read_float                 (FILE *__stream)
{
  int sign=1, readed=0, decimal, total;
  int ch;
  double val=0;

  if (!__stream)
    Quit (_CR, "Specified file not opened");

  // Scip forwarding spaces
  ch=fgetc (__stream);
  while (is_space (ch) && ch!=EOF)
    ch=fgetc (__stream);

  // Get sign of the number
#ifdef ALLOW_FLOAT_POSITIVE_SIGN
  if (ch=='-' || ch=='+')
#else
  if (ch=='-')
#endif
    {
      if (ch=='-')
        sign=-1;
      ch=fgetc (__stream);
    }
  
  // Collect number
  
  decimal=-1; total=0;
  
  while ((ch>='0' && ch<='9') || ch=='.' || ch==',')
    {
      if (ch=='.' || ch==',')
        {
          // Decimal point is already caught
          if (decimal>0)
            Quit (_PE, "Invalid floating-point value.");
          decimal=total;
          readed=0;
          ch=fgetc (__stream);
          continue;
        }

      readed=1;
      val=val*10+ch-'0';
      ch=fgetc (__stream);

      total++;
    }

  if ((!is_space (ch) && ch!=EOF) || !readed)
    Quit (_PE, "Invalid floating-point value.")

  goback (__stream);

  if (decimal<0) return val*sign;

  while (total>decimal)
    {
      val/=10;
      total--;
    }

  return val*sign;
}

void    // Read string from stream
testlib_read_string                (FILE *__stream, char *__buf, int __maxlen)
{
  int len=0;
  int ch;

  if (!__stream)
    Quit (_CR, "Specified file not opened");

  ch=fgetc (__stream);
  
  if (ch==EOF)
    Quit (_PE, "Unexpected end of file");
  
  // Read string
  while (ch!='\n' && ch!='\r' && ch!=EOF && len<__maxlen)
    {
      __buf[len++]=ch;
      ch=fgetc (__stream);
    }

  if (ch=='\r')
    if (fgetc (__stream)!='\n')
      goback (__stream);

  __buf[len]=0;
}

int      // Read char from stream and move pointer t0 next
testlib_read_char                  (FILE *__stream)
{
  int ch;
  
  if (!__stream)
    Quit (_CR, "Specified file not opened");
  
  ch=fgetc (__stream);
  if (ch==EOF)
    Quit (_PE, "No chars to read.");
  return ch;
}

int     // Just return current character in stream
testlib_cur_char                   (FILE *__stream)
{
  int ch;
  
  if (!__stream)
    Quit (_CR, "Specified file not opened");
  
  ch=fgetc (__stream);
  goback (__stream);
  if (ch==EOF)
    Quit (_PE, "No chars to read.");
  return ch;
}

int   // Test for end of file
testlib_eof                        (FILE *__stream)
{
  if (!__stream)
    Quit (_CR, "Specified file not opened");

  int ch=fgetc (__stream);
  int res=ch==EOF;
  goback (__stream);
  return res;
}

int     // Test for end of file
testlib_eoln                       (FILE *__stream)
{
  if (!__stream)
    Quit (_CR, "Specified file not opened");

  int ch=fgetc (__stream);
  int res=ch=='\n' || ch=='\r' || ch==EOF;
//  goback (__stream);
  return res;
}

int     // Seek end of line
testlib_seekeof                    (FILE *__stream)
{
  int ch, res;
  size_t pos;

  if (!__stream)
    Quit (_CR, "Specified file not opened");

  if (feof (__stream))
    return 1;

  pos=fposget (__stream);

  ch=fgetc (__stream);
  while (is_space (ch) && ch!=EOF)
    ch=fgetc (__stream);

  res=ch==EOF;
  
  fposset (__stream, pos);
  
  return res;
}

int    // Seek end of line
testlib_seekeoln                   (FILE *__stream)
{
  int ch, res;
  size_t pos;

  if (!__stream)
    Quit (_CR, "Specified file not opened");

  if (feof (__stream))
    return 1;

  pos=fposget (__stream);

  ch=fgetc (__stream);
  while (is_space (ch) && (ch!='\n' && ch!='\r' && ch!=EOF))
    ch=fgetc (__stream);

  res=ch==EOF || ch=='\r' || ch=='\n';
  
  fposset (__stream, pos);

  return res;
}

void    // Skip characters and move to next line
testlib_next_line                  (FILE *__stream)
{
  int ch;

  if (!__stream)
    Quit (_CR, "Specified file not opened");

  ch=fgetc (__stream);
  while (ch!='\r' && ch!='\n')
    {
      if (ch==EOF) Quit (_PE, "Unexpected end of file");
      ch=fgetc (__stream);
    }
  
  if (ch=='\n')
    {
      ch=fgetc (__stream);
      if (ch==EOF) Quit (_PE, "Unexpected end of file");
      if (ch!='\r') goback (__stream);
    } else
    {
      if (ch==EOF) Quit (_PE, "Unexpected end of file");
      goback (__stream);
    }
}

void    // Skip characterd from charset. Do not generate errors
testlib_skip                       (FILE *__stream, char *__charset)
{
  int ch, i, n;
  int skip_chars[255];

  if (!__stream)
    Quit (_CR, "Specified file not opened");

  memset (skip_chars, 0, sizeof (skip_chars));

  for (i=0, n=strlen (__charset); i<n; i++)
    skip_chars[(int)__charset[i]]=1;

  ch=fgetc (__stream);
  while (ch!=EOF && skip_chars[ch]) ch=fgetc (__stream);

  if (ch!=EOF)
    goback (__stream);
}

////
//

void
testlib_set_output_stream          (FILE *__stream)
{
  out_stream=__stream;
}

////////////////////////////////////////
// Deep-side stuff

static void
quit_message_prefix                (int __errno)
{
  if (__errno!=_OK)
    {
      PRINT ("\33[31;1m");
      if (__errno==_WA) PRINT ("WA");
      if (__errno==_PE) PRINT ("PE");
      if (__errno==_CR) PRINT ("CR");
      PRINT ("\33[0m");
    } else
      PRINT ("\33[32;1mOK\33[0m");
}

static void
quit_message                       (int __errno, char *__desc)
{
  quit_message_prefix (__errno);
  if (strcmp (__desc, ""))
    PRINT ("  %s", __desc);
  PRINT ("\n");

  if (!silent && __errno!=_OK)
    {
//      PRINT ("\33[10;50;11;1000]\7");
      usleep (200000);
    }
}

void
testlib_quit                       (int __errno, char *__desc)
{
  if (__errno==_OK)
    {
      if (!out_stream)
        {
          quit_message (_CR, "Output file not assigned.");
          exit (_CR);
        }
      if (testlib_seekeof (out_stream))
        {
          quit_message (__errno, __desc);
          exit (__errno);
        }
      quit_message (_WA, "Extra information in tail of the output file.");
      exit (_WA);
    }
  quit_message (__errno, __desc);
  exit (__errno);
}

void
testlib_silent                     (int __val)
{
  silent=__val;
}
