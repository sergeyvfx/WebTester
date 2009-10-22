/**
 * WebTester Server - server of on-line testing system
 *
 * Uniques for LibRUN
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _RUN_UNIQUE_H_
#define _RUN_UNIQUE_H_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

int
run_unique_init (void);

void
run_unique_done (void);

int
run_unique_alloc (void);

void
run_unique_release (int __uid);

END_HEADER

#endif
