/*
 *
 * ================================================================================
 *  scheduler.h
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _scheduler_h_
#define _scheduler_h_

#include <libwebtester/types.h>

typedef int (*scheduler_callback) (void *__data);

int             // Initialize scheduler stuff
scheduler_init                     (void);

void            // Uninitialize scheduler stuff
scheduler_done                     (void);

int             // Add task to scheduler
scheduler_add                      (scheduler_callback __callback, void *__data, __u64_t __interval);

void            // Remove task from scheduler
scheduler_remove                   (scheduler_callback __callback);

void
scheduler_overview                 (void);

#endif
