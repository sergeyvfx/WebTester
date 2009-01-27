/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_QUEUE_H_
#define _WT_QUEUE_H_

#include "autoinc.h"

BEGIN_HEADER

#include <libwebtester/types.h>
#include <libwebtester/dynastruc.h>

/* Default size of queue */
#define QUEUE_SIZE   4

/* Default count of unpacking tasks */
#define UNPACK_COUNT  1

#define QUEUE_AUTOSTART 0

/* Initialize queue stuff */
int
wt_queue_init (void);

/* Uninitialize queue stuff */
void
wt_queue_done (void);

/* Update queue */
int
wt_queue_update (void);

/* Unpack parameters for some tasks */
void
wt_queue_unpack (void);

/* Free all cells of queue */
void
wt_queue_free (void);

/* Size of queue */
long
wt_queue_size (void);

/* Current length of queue */
long
wt_queue_length (void);

/* Is queue empty? */
BOOL
wt_queue_empty (void);

/* Get queue descriptor */
dynastruc_t*
wt_queue (void);

/* Start queue */
void
wt_queue_start (void);

/* Stop queue */
void
wt_queue_stop (void);

END_HEADER

#endif
