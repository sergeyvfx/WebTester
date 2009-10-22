/**
 * WebTester Server - server of on-line testing system
 *
 * Scheduling
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef scheduler_h
#define scheduler_h

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/types.h>

typedef int (*scheduler_callback) (void *__data);

/* Initialize scheduler stuff */
int
scheduler_init (void);

/* Uninitialize scheduler stuff */
void
scheduler_done (void);

/* Add task to scheduler */
int
scheduler_add (scheduler_callback __callback, void *__data,
               __u64_t __interval);

/* Remove task from scheduler */
void
scheduler_remove (scheduler_callback __callback);

/* Overview scheduler tasks */
void
scheduler_overview (void);

END_HEADER

#endif
