/*
 * ================================================================================
 *  unique.h - part of the LibRUN
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _run_unique_h_
#define _run_unique_h_

int
run_unique_init                    (void);

void
run_unique_done                    (void);

int
run_unique_alloc                   (void);

void
run_unique_release                 (int __uid);

#endif
