/*
 * ================================================================================
 *  util.c - part of the TestLib
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "util.h"

size_t
fsize                              (FILE *__stream)
{
  size_t result, pos;
  pos=ftell (__stream);
  fseek (__stream, 0, SEEK_END);
  result=ftell (__stream);
  fseek (__stream, pos, SEEK_SET);
  return result;
}

size_t
fposget                            (FILE *__stream)
{
  return ftell (__stream);
}

void
fposset                            (FILE *__stream, size_t __pos)
{
  fseek (__stream, __pos, SEEK_SET);
}

int
is_space                           (char __ch)
{
  if (__ch==' '||__ch=='\n'||__ch=='\t'||__ch=='\r')
    return 1;
  return 0;
}
