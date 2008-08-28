/*
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <testlib++.h>

void
Check                              ()
{
  string a, b;
  while (!ans.Eof ())
    {
      if (ouf.ReadString ()!=ans.ReadString ())
        Quit (_WA, "String mistmatch");
    }
  if (!ouf.Eof ()) Quit (_WA, "Extra information in output file");
  Quit (_OK, "OK");
}
