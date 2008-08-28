/*
 *
 * ================================================================================
 *  pipe.h - part of the WebTester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_pipe_h_
#define _wt_pipe_h_

#include "common.h"

BOOL
pipe_init                          (void);

void
pipe_done                          (void);

void
pipe_set_socket                    (int __self);

int
pipe_get_socket                    (void);

#endif

