/*
 *
 * ================================================================================
 *  stat.h - part of the WebTester Server Server
 * ================================================================================
 *
 *  Statistics stuff.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_stat_h_
#define _wt_stat_h_

#include <libwebtester/flexval.h>

int
wt_stat_init                       (void);

void
wt_stat_done                       (void);

////
//

void
wt_stat_set_int                    (char *__var, long __val);

void
wt_stat_set_float                  (char *__var, double __val);

void
wt_stat_set_string                 (char *__var, char *__val);

void
wt_stat_set_array                  (char *__var, flex_value_t **__val);

void
wt_stat_set_desc                   (char *__var, char *__desc);

#endif
