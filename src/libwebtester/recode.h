/**
 * WebTester Server - server of on-line testing system
 *
 * This module contains utf-8 stuff and some other stuff to work with charsets.
 * (Based on the source of X MultiMedia Systems)
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _RECODE_H_
#define _RECODE_H_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

/* Get current system character set */
const char*
get_current_charset (void);

/* Recode string from one character set to another */
char*
recode (const char *__string, const char *__from, const char *__to);

/* Recode file content from one character set to another */
void
recode_file (const char *__filename, const char *__from, const char *__to);

#ifdef USE_ENCA
/* Guess buffer's character set */
const char*
guess_buffer_cp (const char *__buffer, size_t __buflen,
                 const char *__preferred_language, const char *__fallback);

/* Guess file's character set */
const char*
guess_file_cp (const char *__filename, const char *__preferred_language,
               const char *__fallback);

#endif

END_HEADER

#endif
