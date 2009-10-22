/**
 * WebTester Server - server of on-line testing system
 *
 * String library. Includes some powerfull
 * functions to work with strings.
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */


#ifndef _strlib_h_
#define _strlib_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

/* Convert string to upper case */
void
strupr (const char *__src, char *__dest);

/* Convert string to lower case */
void
strlowr (const char *__src, char *__dest);

/* Get last occurence of character in string */
int
strlastchar (const char *__str, char __ch);

/* Extract substring from string */
void
strsubstr (const char *__src, int __start, int __len, char *__out);

/* Split string by separator into array of string */
long
explode (const char *__s, const char *__separator, char ***__out);

/* Free array from explode */
void
free_explode_data (char **__self);

/* Add slashed before `dangerous` characters */
void
addslashes (const char *__self, char *__out);

/* Strinp slashes from string */
void
stripslashes (const char *__self, char *__out);

/* Append string to array of string */
void
strarr_append (char ***__arr, const char *__s, int *__count);

/* Free array f string */
void
strarr_free (char **__arr, int __count);

/* Realloc memory allocated for string */
char*
realloc_string (const char *__s, int __delta);

/* Trim space characters from beginning and ending of string */
void
trim (const char *__self, char *__out);

END_HEADER

#endif
