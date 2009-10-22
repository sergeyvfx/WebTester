/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_MAINLOOP_H_
#define _WT_MAINLOOP_H_

#include "autoinc.h"

BEGIN_HEADER

#define MAINLOOP_QUEUE_UPDATE_INTERVAL  1.0  /* secs */
#define MAINLOOP_BELTS_UPDATE_INTERVAL  1.0  /* secs */
#define MAINLOOP_UPLOAD_INTERVAL        4.0  /* secs */

#define MAINLOOP_UNPACK_INTERVAL        1.0  /* secs */

#define SCHEDULER_INTERVAL              0.5  /* secs */

#define MAINLOOP_DELAY                  0.2  /* secs */

/* Initialize MainLoop stuff */
int
wt_mainloop_init (void);

/* Uninitialize MainLoop stuff */
void
wt_mainloop_done (void);

END_HEADER

#endif
