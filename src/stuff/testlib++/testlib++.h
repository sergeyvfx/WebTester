/**
 * WebTester Server - server of on-line testing system
 *
 * C++ interface under testlib
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_TETSLIBPP_H_
#define _WT_TETSLIBPP_H_

#include <stdio.h>
#include <string>
#include <iostream>
#include <testlib.h>

using namespace std;

class CTestlibFile;

extern CTestlibFile inf, ouf, ans;

#define TESTLIB_MAX_STRLEN 65536

/****
 * Some powerful defenitions
 */

class CTestlibFile
{
public:
  /*  Constructors*/
  CTestlibFile (void);
  CTestlibFile (char *__fn);

  /* Destructor */
  ~CTestlibFile (void);

  int Assign (char *__fn);

  /****
   * User's backend
   */

  int    ReadInteger (void); /* Read integer from file */
  long   ReadLongint (void); /* Read longint from file */
  double ReadFloat (void);   /* Read double from file */
  double ReadReal (void);    /* Read real from file */
  string ReadString (void);  /* Read string from file */
  int    ReadChar (void);    /* Read char from file and move to next */
  int    CurChar (void);     /* Just return current char */

  int  Eoln (void);     /* Check for end of line */
  int  SeekEoln (void); /* Seek for end of file */
  void NextLine (void); /* Move to next line */

  /* Skip characters from charset. Do not generate errors */
  void Skip (const char *__charset);

  int SeekEof (void); /* Seek end of file */
  int Eof (void);     /* Check for end of file */

  FILE* GetStream (void);

protected:
  /* Some internal stuff */
  int OpenStream (const char *__fn);
  void CloseStream (void);

protected:
  FILE *stream;
};

/* Initialize testlib */
int
Testlib_Init (int __argc, char **__argv);

/* Uninitialize testlib */
void
Testlib_Done (void);

#ifndef TESTLIB_SO
void Check (void);

int
main (int __argc, char **__argv)
{
  Testlib_Init (__argc, __argv);
  Check ();
  Testlib_Done ();
  return 0;
}
#endif

#endif
