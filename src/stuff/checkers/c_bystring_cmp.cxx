/**
 * WebTester Server - server of on-line testing system
 *
 * Checker for comparing files line-by-line
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
  while (!ans.Eof ())
    {
      if (ouf.ReadString () != ans.ReadString ())
        {
          Quit (_WA, "String mistmatch");
        }
    }

  if (!ouf.Eof ())
    {
      Quit (_WA, "Extra information in output file");
    }

  Quit (_OK, "OK");
}
