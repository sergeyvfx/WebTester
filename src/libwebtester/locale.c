/*
 *
 * ================================================================================
 *  locale.c
 * ================================================================================
 *
 *  Locale stuff
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "locale.h"
#include "utf8.h"

char*
recode_database_data            (const char *__string)
{
  return recode (
                  __string,
                  "UTF-8", // ### !! FIXME !! ###
                  "UTF-8"
                );
}
