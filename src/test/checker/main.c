/*
 * ================================================================================
 *  main.c - part of the checker
 * ================================================================================
 *
 *  Sample checker.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <testlib.h>
#include <string.h>

void
Check                              (void)
{
  char a[1024], b[1024];
  while (!testlib_eof (ans))
    {
      testlib_read_string (ouf, a, 1024);
      testlib_read_string (ans, b, 1024);
      if (strcmp (a, b))
        Quit (_WA, "String mistmatch");
    }
  if (!testlib_eof (ouf))
    Quit (_WA, "Extra information in output file");
  Quit (_OK, "");
}
