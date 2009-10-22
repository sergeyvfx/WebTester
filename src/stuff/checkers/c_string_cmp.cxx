/**
 * WebTester Server - server of on-line testing system
 *
 * Checker for comparing two long string from output and answer files
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <testlib++.h>

void
Check ()
{
  if (ouf.ReadString () != ans.ReadString ())
    {
      Quit (_WA, "String mistmatch");
    }

  Quit (_OK, "OK");
}
