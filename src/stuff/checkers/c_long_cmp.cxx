/*
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <testlib++.h>

void
Check                              ()
{
  if (ouf.ReadLongint ()!=ans.ReadLongint ())
    Quit (_WA, "Value mistmatch");
  Quit (_OK, "OK");
}
