/*
 * ================================================================================
 *  testlib.cpp - part of the TestLib++
 * ================================================================================
 *
 *  C++ interface under testlib
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_testlibpp_h_
#define _wt_testlibpp_h_

#include <stdio.h>
#include <string>
#include <iostream>
#include <testlib.h>

using namespace std;

class CTestlibFile;

extern CTestlibFile inf, ouf, ans;

#define TESTLIB_MAX_STRLEN 65536

////////////////////////////////////////
// Some powerful defenitions

class CTestlibFile
{
public:
  /// Constructors
  CTestlibFile (void);
  CTestlibFile (char *__fn);

  /// Destructor
  ~CTestlibFile (void);

  int Assign    (char *__fn);

  ////////
  // User's backend

  int     ReadInteger (void); // Read integer from file
  long    ReadLongint (void); // Read longint from file
  double  ReadFloat   (void); // Read double from file
  string  ReadString  (void); // Read string from file
  int     ReadChar    (void); // Read char from file and move to next
  int     CurChar     (void); // Just return current char
  
  int     Eoln        (void); // Check for end of file
  int     SeekEoln    (void); // Seek for end off file
  void    NextLine    (void); // Move to next line

  // Skip characters from charset. Do not generate errors
  void    Skip        (char *__charset);

  int     SeekEof     (void); // Seek end of file
  int     Eof         (void); // Check for end of file

  FILE*  GetStream   (void);

protected:
  /// Some internal stuff
  int  OpenStream  (char *__fn);
  void CloseStream (void);

protected:
  FILE *stream;
};

int
Testlib_Init                       (int __argc, char **__argv);

void
Testlib_Done                       (void);

#ifndef TESTLIB_SO
  void Check (void);
  int main (int __argc, char **__argv)
    {
      Testlib_Init (__argc, __argv);
      Check ();
      Testlib_Done ();
      return 0;
    }
#endif

#endif
