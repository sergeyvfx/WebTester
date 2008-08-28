/*
 *
 * ================================================================================
 *  transport.h - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_webiface_transport_h_
#define _wt_webiface_transport_h_

#include <libwebtester/dynastruc.h>
#include <libwebtester/network-soup.h>
#include <libwebtester/types.h>
#include <webtester/task.h>

int             // Initialize 
webiface_transport_init            (void);

void
webiface_transport_done            (void);

int             // Delete task from testing queue
webiface_delete_task               (wt_task_t *__self);

int             // Restore task in testing queue
webiface_restore_task              (wt_task_t *__self);

int             // Get detailed task information
webiface_get_task                  (wt_task_t *__self);

int             // Return tested task to WebInterface
webiface_put_soution               (wt_task_t *__self);

int             // Reset status of half-tested tasks
webiface_reset_status              (void);

int             // Recieve list of untested problems
webiface_get_task_list             (dynastruc_t *__tasklist, int __queue_size);

void
webiface_prepare_url               (char *__self, char *__out);

http_message_t* // Send simple message with URL
webiface_send_message              (char *__url);

DWORD
webiface_bytes_send                (void);

DWORD
webiface_bytes_recv                (void);

#endif
