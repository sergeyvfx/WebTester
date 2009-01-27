/**
 * WebTester Server - server of on-line testing system
 *
 * C++ interface under testlib
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#define TESTLIB_SO

#include "testlib++.h"
#include <stdlib.h>

CTestlibFile inf, ouf, ans;

/****
 * Constructors
 */

CTestlibFile::CTestlibFile (void)
{
  this->stream = 0;
}

CTestlibFile::CTestlibFile (char *__fn)
{
  this->stream = 0;
  OpenStream (__fn);
}

/****
 * Destructor
 */

CTestlibFile::~CTestlibFile (void)
{
  CloseStream ();
}

/****
 *
 */
int
CTestlibFile::Assign (char *__fn)
{
  return this->OpenStream (__fn);
}

/****
 * User's backend
 */

/**
 * Read integer from file
 *
 * @return read integer value
 */
int
CTestlibFile::ReadInteger (void)
{
  return testlib_read_integer (this->GetStream ());
}

/**
 * Read longint from file
 *
 * @return read longint value
 */
long
CTestlibFile::ReadLongint (void)
{
  return testlib_read_longint (this->GetStream ());
}

/**
 * Read float from file
 *
 * @return read float value
 */
double
CTestlibFile::ReadFloat (void)
{
  return testlib_read_float (this->GetStream ());
}

/**
 * Read string from file
 *
 * @return read string
 */
string
CTestlibFile::ReadString (void)
{
  char buf[TESTLIB_MAX_STRLEN + 1];
  string a;
  testlib_read_string (this->GetStream (), buf, TESTLIB_MAX_STRLEN);
  a = buf;
  return a;
}

/**
 * Read character from file
 *
 * @return read chat core
 */
int
CTestlibFile::ReadChar (void)
{
  return testlib_read_char (this->GetStream ());
}

/**
 * Just return current char
 *
 * @return current char core
 */
int
CTestlibFile::CurChar (void)
{
  return testlib_cur_char (this->GetStream ());
}

/**
 * Move to next line
 */
void
CTestlibFile::NextLine (void)
{
  testlib_next_line (this->GetStream ());
}

/**
 * Skip characters from charset. Do not generate errors
 *
 * @param __charset - set of characters t skip
 */
void
CTestlibFile::Skip (const char *__charset)
{
  testlib_skip (this->GetStream (), __charset);
}

/**
 * Seek for end of line
 *
 * @return non-zero if EOLn found, zero otherwise
 */
int
CTestlibFile::SeekEoln (void)
{
  return testlib_seekeoln (this->GetStream ());
}

/**
 * Check for end of line
 *
 * @return non-zero in case of EOLn, zero otherwise
 */
int
CTestlibFile::Eoln (void)
{
  return testlib_eoln (this->GetStream ());
}

/**
 * Seek for end of file
 *
 * @return non-zero if EOF found, zero otherwise
 */
int
CTestlibFile::SeekEof (void)
{
  return testlib_seekeof (this->GetStream ());
}

/**
 * Check for end of file
 *
 * @return non-zero in case of EOF, zero otherwise
 */
int
CTestlibFile::Eof (void)
{
  return testlib_eof (this->GetStream ());
}

/****
 * Some internal stuff
 */

/**
 * Open stream for reading from
 * @param __fn - name of file to open
 * @return zero on error, non-zero otherwise
 */
int
CTestlibFile::OpenStream (const char *__fn)
{
  this->stream = fopen (__fn, "r");

  if (!this->stream)
    {
      return 0;
    }

  return 1;
}

/**
 * Close opened stream
 */
void
CTestlibFile::CloseStream (void)
{
  if (this->GetStream ())
    {
      fclose (this->GetStream ());
    }
}

/**
 * Get stream descriptor
 *
 * @return stream descriptor
 */
FILE*
CTestlibFile::GetStream (void)
{
  return this->stream;
}

/****
 *
 */

/**
 * Initialize TestLib
 *
 * @param _argc - count of coomand line arguments
 * @param _argv - values of coomand line arguments
 */
int
Testlib_Init (int __argc, char **__argv)
{
  /* Check for arguments */
  if (__argc < 4)
    {
      char usage[1024];
      sprintf (usage, "Usage: %s <input file> <output file> "
                      "<answer file> [-s]", __argv[0]);
      Quit (-1, usage);
    }

  if (__argc > 4 && !strcmp (__argv[4], "-s"))
    {
      testlib_silent (1);
    }

  /* Assign streams */
  inf.Assign (__argv[1]);
  ouf.Assign (__argv[2]);
  ans.Assign (__argv[3]);

  if (!ouf.GetStream ())
    {
      Quit (_PE, "Output file not found");
    }

  testlib_set_output_stream (ouf.GetStream ());

  return 1;
}

/**
 * Uninitialize TestLib
 */
void
Testlib_Done (void)
{
  Quit (_OK, "");
}
