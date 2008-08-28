/*
 *
 * ================================================================================
 *  queue.c - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_queue_h_
#define _wt_queue_h_

#include <libwebtester/types.h>
#include <libwebtester/dynastruc.h>

// Default size of queue
#define  QUEUE_SIZE   4
// Default count of unpacking tasks
#define UNPACK_COUNT  1

////
//

int             // Initialize queue stuff
wt_queue_init                      (void);

void            // Uninitialize queue stuff
wt_queue_done                      (void);

int
wt_queue_update                    (void);

void            // Unpack parameters for some tasks
wt_queue_unpack                    (void);

void            // Free all cells of queue
wt_queue_free                      (void);

long            // Size of queue
wt_queue_size                      (void);

long            // Current length oq queue
wt_queue_length                    (void);

BOOL
wt_queue_empty                     (void);

dynastruc_t*
wt_queue                           (void);

#endif
