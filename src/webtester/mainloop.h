/*
 *
 * ================================================================================
 *  mainloop.h - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_mainloop_h_
#define _wt_mainloop_h_

#define MAINLOOP_QUEUE_UPDATE_INTERVAL  1.0  /* secs */
#define MAINLOOP_BELTS_UPDATE_INTERVAL  1.0  /* secs */
#define MAINLOOP_UPLOAD_INTERVAL        4.0  /* secs */

#define MAINLOOP_UNPACK_INTERVAL        1.0  /* secs */

#define SCHEDULER_INTERVAL              0.5  /* secs */

#define MAINLOOP_DELAY                  0.2  /* secs */

int             // Initialize MainLoop stuff
wt_mainloop_init                   (void);

void            // Uninitialize MainLoop stuff
wt_mainloop_done                   (void);

#endif
