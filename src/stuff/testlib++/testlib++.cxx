/*
 * ================================================================================
 *  testlib.cxx - part of the TestLib++
 * ================================================================================
 *
 *  C++ interface under testlib
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#define TESTLIB_SO

#include "testlib++.h"
#include <stdlib.h>

CTestlibFile inf, ouf, ans;

////////////////////////////////////////
// Constructors

CTestlibFile::CTestlibFile         (void)
{
  this->stream=0;
}

CTestlibFile::CTestlibFile         (char *__fn)
{
  this->stream=0;
  OpenStream (__fn);
}

////////////////////////////////////////
// Destructor

CTestlibFile::~CTestlibFile        (void)
{
  CloseStream ();
}

////////////////////////////////////////
//

int
CTestlibFile::Assign               (char *__fn)
{
  return this->OpenStream (__fn);
}

////////////////////////////////////////
// User's backend

int
CTestlibFile::ReadInteger          (void)
{
  return testlib_read_integer (this->GetStream ());
}

long
CTestlibFile::ReadLongint          (void)
{
  return testlib_read_longint (this->GetStream ());
}

double
CTestlibFile::ReadFloat            (void)
{
  return testlib_read_float (this->GetStream ());
}

string
CTestlibFile::ReadString           (void)
{
  char buf[TESTLIB_MAX_STRLEN+1];
  string a;
  testlib_read_string (this->GetStream (), buf, TESTLIB_MAX_STRLEN);
  a=buf;
  return a;
}

int
CTestlibFile::ReadChar             (void)
{
  return testlib_read_char (this->GetStream ());
}

int
CTestlibFile::CurChar              (void)
{
  return testlib_cur_char (this->GetStream ());
}

void
CTestlibFile::NextLine             (void)
{
  testlib_next_line (this->GetStream ());
}

void
CTestlibFile::Skip                 (char *__charset)
{
  testlib_skip (this->GetStream (), __charset);
}

////
//

int
CTestlibFile::SeekEoln             (void)
{
  return testlib_seekeoln (this->GetStream ());
}

int
CTestlibFile::Eoln                 (void)
{
  return testlib_eoln (this->GetStream ());
}

int
CTestlibFile::SeekEof              (void)
{
  return testlib_seekeof (this->GetStream ());
}

int
CTestlibFile::Eof                  (void)
{
  return testlib_eof (this->GetStream ());
}

////////////////////////////////////////
// Some internal stuff

int
CTestlibFile::OpenStream           (char *__fn)
{
  this->stream=fopen (__fn, "r");
  if (!this->stream)
    return 0;
  return 1;
}

void
CTestlibFile::CloseStream          (void)
{
  if (this->GetStream ())
    fclose (this->GetStream ());
}

FILE*
CTestlibFile::GetStream            (void)
{
  return this->stream;
}
////////////////////////////////////////
//

int
Testlib_Init                       (int __argc, char **__argv)
{
  // Check for arguments
  if (__argc<4)
    {
      char usage[1024];
      sprintf (usage, "Usage: %s <input file> <output file> <answer file> [-s]", __argv[0]);
      Quit (-1, usage);
    }

  if (__argc>4 && !strcmp (__argv[4], "-s"))
    testlib_silent (1);

  // Assign streams
  inf.Assign (__argv[1]);
  ouf.Assign (__argv[2]);
  ans.Assign (__argv[3]);

  if (!ouf.GetStream ())
    Quit (_PE, "Output file not found");

  testlib_set_output_stream (ouf.GetStream ());

  return 1;
}

void
Testlib_Done                       (void)
{
  Quit (_OK, "");
}
