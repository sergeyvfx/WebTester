/*
 * ================================================================================
 *  util.h - part of the TestLib
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_testlib_util_h_
#define _wt_testlib_util_h_

#include <stdio.h>

size_t
fsize                              (FILE *__stream);

size_t
fposget                            (FILE *__stream);

void
fposset                            (FILE *__stream, size_t __pos);

int
is_space                           (char __ch);

#endif
