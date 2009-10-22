/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_IPC_H_
#define _WT_IPC_H_

#include "autoinc.h"

BEGIN_HEADER

#define WT_IPC_HOST   "*"
#define WT_IPC_PORT   13666
#define WT_IPC_DELAY  0.2 /* secs */

#define WT_IPC_INCORRECT_LOGIN_DELAY           300 /* msecs */
#define WT_IPC_BLACKLIST_RESET_TIMEOUT         (24*60*60) /* secs */
#define WT_IPC_BLACKLIST_TRIES_BEFORE_LOCK     5
#define WT_IPC_BLACKLIST_TIME_FOR_COUNT_TRIES  60 /* secs */
#define WT_IPC_BLACKLIST_BL_ON_TRIES_EXPIRED   1
#define WT_IPC_BLACKLIST_REVIEW_LOGIN_INFO_INTERVAL (300*USEC_COUNT)

/* Initialize IPC stuff */
int
wt_ipc_init (void);

/* Uninitialize IPC stuff */
void
wt_ipc_done (void);

/* Get delay in incorrect login  */
int
wt_ipc_get_incorrect_login_delay (void);

/* Is IPC supported? */
int
wt_ipc_supported (void);

/* Initialize IPC builtin */
int
wt_ipc_builtin_init (void);

/* Uninitialize IPC builtin */
void
wt_ipc_builtin_done (void);

END_HEADER

#endif
