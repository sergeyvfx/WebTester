/*
 *
 * ================================================================================
 *  stat.h - part of the WebTester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _stat_h_
#define _stat_h_

#define STAT_FULL_UPDATE_INTERVAL 10.0 /* secs */

int
stat_init                          (void);

void
stat_done                          (void);

void
stat_update_monitors               (void);

void
stat_reset_monitors                (void);

void
stat_set_socket                    (int __self);

int
stat_get_socket                    (void);

void
stat_on_disconnect                 (void);

#endif
