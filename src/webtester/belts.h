/**
 * WebTester Server - server of on-line testing system
 *
 * Belts' stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_BELTS_H_
#define _WT_BELTS_H_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/dynastruc.h>

#include <webtester/task.h>

/* Default size of belts */
#define BELTS_SIZE   4

#define BELTS_AUTOSTART 0

/* Interval of accumulator overview */
#define ACC_OVERVIEW_INT (4 * USEC_COUNT)

/* Initialize belts' stuff */
int
wt_belts_init (void);

/* Uninitialize belts size */
void
wt_belts_done (void);

/* Max avaliable size of belts */
int
wt_belts_size (void);

/* Free all cells of belts */
void
wt_belts_free (void);

/* Check for task is in belts */
BOOL
wt_task_in_belts (wt_task_t *__self);

/* Renew status and put new tasks */
int
wt_belts_update (void);

/* Is belts empty? */
BOOL
wt_belts_empty (void);

/* Is belts full? */
BOOL
wt_belts_full (void);

/* Current length of belts */
long
wt_belts_length (void);

/* Start belts */
void
wt_belts_start (void);

/* Stop belts */
void
wt_belts_stop (void);

/* Is beltsstatus is changed */
void
wt_belts_status_changed (void);

END_HEADER

#endif
