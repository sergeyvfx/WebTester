/**
 * WebTester Server - server of on-line testing system
 *
 * Some usefull procedures
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef util_h
#define util_h

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <sys/time.h>

#define leapyear(year)((year % 100 ? year : year / 100) % 4 == 0)

/* Check is string a number */
int
is_number (const char *__self);

/* Check is string an integer number */
int
is_integer (const char *__self);

/* Drop trilling zeroes from number in string */
void
drop_triling_zeroes (char *__self);

/* System parser */
int
fname (const char *__full, char *__out);

/* Get name of a directory */
int
dirname (const char *__full, char *__out);

/* Drop extension from name of file */
int
dropextension (const char *__fn, char *__out);

/* Get file's extension */
int
getextension (const char *__fn, char *__out);

/* Launch command */
int
sys_launch (const char *__args, ...);

/* Get current time */
timeval_t
now (void);

timeval_t
timedist (timeval_t __from, timeval_t __to);

/* Get current time */
timeval_t
timedistnow (timeval_t __from);

/* Compare timeval and milliseconds */
int
tv_usec_cmp (timeval_t __tv, __u64_t __usec);

/* Prepare argument for command line */
void
prepare_cmdarg (const char *__src, char *__dst);

/* Get user's id by its name */
long
uid_by_name (const char *__name);

/* Get group's id by its name */
long
gid_by_name (const char *__name);

/* Format current date with given format string */
void
get_datetime_strf (char *__out, int __size, const char *__format);

/* Check is given string is a truth value */
int
is_truth (const char *__self);

/* Floating absolutely value */
double
fabs (double __self);

/* Get sign of number */
double
sign (double __self);

/* Parse range string */
int
parse_range (const char *__range, long *__from, long *__to);

END_HEADER

#endif
