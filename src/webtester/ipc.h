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

int             // Initialize IPC stuff
wt_ipc_init                        (void);

void            // Uninitialize IPC stuff
wt_ipc_done                        (void);

#endif
