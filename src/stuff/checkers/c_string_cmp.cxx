/*
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <testlib++.h>

void
Check                              ()
{
  if (ouf.ReadString ()!=ans.ReadString ())
    Quit (_WA, "String mistmatch");
  Quit (_OK, "OK");
}
