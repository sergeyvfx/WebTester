/*
 *
 * ================================================================================
 *  ipc.h - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_ipc_h_
#define _wt_ipc_h_

#define WT_IPC_HOST   "*"
#define WT_IPC_PORT   13666
#define WT_IPC_DELAY  0.2 /* secs */

#define WT_IPC_INCORRECT_LOGIN_DELAY           300 /* msecs */
#define WT_IPC_BLACKLIST_RESET_TIMEOUT         (24*60*60) /* secs */
#define WT_IPC_BLACKLIST_TRIES_BEFORE_LOCK     5
#define WT_IPC_BLACKLIST_TIME_FOR_COUNT_TRIES  60 /* secs */
#define WT_IPC_BLACKLIST_BL_ON_TRIES_EXPIRED   1
#define WT_IPC_BLACKLIST_REVIEW_LOGIN_INFO_INTERVAL (300*USEC_COUNT)

int             // Initialize IPC stuff
wt_ipc_init                        (void);

void            // Uninitialize IPC stuff
wt_ipc_done                        (void);

int
wt_ipc_get_incorrect_login_delay   (void);

int
wt_ipc_supported                   (void);

int
wt_ipc_builtin_init                (void);

void
wt_ipc_builtin_done                (void);

#endif
