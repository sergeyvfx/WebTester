/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_TESTLIB_UTIL_H_
#define _WT_TESTLIB_UTIL_H_

#include <stdio.h>

/* Size of stream */
size_t
fsize (FILE *__stream);

/* Get current position in stream */
size_t
fposget (FILE *__stream);

/* Set current position in stream */
void
fposset (FILE *__stream, size_t __pos);

/* Check in character is pace */
int
is_space (char __ch);

#endif
