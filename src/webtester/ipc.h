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

#define WT_IPC_HOST "*"
#define WT_IPC_PORT 13666
#define WT_IPC_DELAY  0.2 /* secs */

int             // Initialize IPC stuff
wt_ipc_init                        (void);

void            // Uninitialize IPC stuff
wt_ipc_done                        (void);

////
//

int
wt_ipc_help                        (int __argc, char **__argv);

int
wt_ipc_login                       (int __argc, char **__argv);

int
wt_ipc_logout                      (int __argc, char **__argv);

int
wt_ipc_stat                        (int __argc, char **__argv);

int
wt_ipc_tail                        (int __argc, char **__argv);

int
wt_ipc_uptime                      (int __argc, char **__argv);

#endif
