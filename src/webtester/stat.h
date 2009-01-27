/**
 * WebTester Server - server of on-line testing system
 *
 * Statistics stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_STAT_H_
#define _WT_STAT_H_

#include "autoinc.h"

BEGIN_HEADER

#include <libwebtester/flexval.h>

/* Initialize stat stuff */
int
wt_stat_init (void);

/* Uninitialize stat stuff */
void
wt_stat_done (void);

/* Set integer stat variable */
void
wt_stat_set_int (const char *__var, long __val);

/* Set float stat variable */
void
wt_stat_set_float (const char *__var, double __val);

/* Set string stat variable */
void
wt_stat_set_string (const char *__var, const char *__val);

/* Set array stat variable */
void
wt_stat_set_array (const char *__var, flex_value_t **__val);

END_HEADER

#endif
