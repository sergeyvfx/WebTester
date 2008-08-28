/*
 *
 * ================================================================================
 *  belts,h
 * ================================================================================
 *
 *  Belts' stuff
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_belts_h_
#define _wt_belts_h_

#include <libwebtester/smartinclude.h>
#include <libwebtester/dynastruc.h>

#include <webtester/task.h>

// Default size of belts
#define BELTS_SIZE   4

#define BELTS_AUTOSTART 0

int             // Max avaliable size of belts
wt_belts_size                      (void);

int             // Initialize belts' stuff
wt_belts_init                      (void);

void            // Uninitialize belts size
wt_belts_done                      (void);

void            // Free all cells of belts
wt_belts_free                      (void);

BOOL            // Check for task is in belts
wt_task_in_belts                   (wt_task_t *__self);

int             // Renew status and put new tasks
wt_belts_update                    (void);

BOOL            // Is belts empty?
wt_belts_empty                     (void);

BOOL            // Is belts full?
wt_belts_full                      (void);

long            // Current length of belts
wt_belts_length                    (void);

void
wt_belts_start                     (void);

void
wt_belts_stop                      (void);

#ifndef _wt_task_h
void
wt_task_status_changed             (void);
#endif

#endif
