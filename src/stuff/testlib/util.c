/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "util.h"

/**
 * Size of stream
 *
 * @param __stream - stream ot measure
 * @return zie of strem
 */
size_t
fsize (FILE *__stream)
{
  size_t result, pos;
  pos = ftell (__stream);
  fseek (__stream, 0, SEEK_END);
  result = ftell (__stream);
  fseek (__stream, pos, SEEK_SET);
  return result;
}

/**
 * Get current position in stream
 *
 * @param __stream - stream to get current position from
 * @return current position in stream
 */
size_t
fposget (FILE *__stream)
{
  return ftell (__stream);
}

/**
 * Set current position in stream
 *
 * @param __stream - stream to set current position in
 * @param __pos - new current position in stream
 */
void
fposset (FILE *__stream, size_t __pos)
{
  fseek (__stream, __pos, SEEK_SET);
}

/**
 * Check in character is pace
 *
 * @param __ch - character to test
 * @return non-zero if character is space, zero otherwise
 */
int
is_space (char __ch)
{
  if (__ch == ' ' || __ch == '\n' || __ch == '\t' || __ch == '\r')
    {
      return 1;
    }

  return 0;
}
