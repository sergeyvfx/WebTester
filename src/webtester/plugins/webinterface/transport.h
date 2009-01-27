/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_WEBIFACE_TRANSPORT_H_
#define _WT_WEBIFACE_TRANSPORT_H_

#include <libwebtester/dynastruc.h>
#include <libwebtester/network-soup.h>
#include <libwebtester/types.h>
#include <webtester/task.h>

/* Initialize transporting */
int
webiface_transport_init (void);

/* Uninitialize transporting */
void
webiface_transport_done (void);

/* Delete task from testing queue */
int
webiface_delete_task (wt_task_t *__self);

/* Restore task in testing queue */
int
webiface_restore_task (wt_task_t *__self);

/* Get detailed task information */
int
webiface_get_task (wt_task_t *__self);

/* Return tested task to WebInterface */
int
webiface_put_soution (wt_task_t *__self);

/* Reset status of half-tested tasks */
int
webiface_reset_status (void);

/* Recieve list of untested problems */
int
webiface_get_task_list (dynastruc_t *__tasklist, int __queue_size);

/* Prepare URL to send */
void
webiface_prepare_url (const char *__self, char *__out);

/* Send simple message with URL */
http_message_t*
webiface_send_message (const char *__url);

/* Get number of bytes send */
DWORD
webiface_bytes_send (void);

/* Get number of bytes received */
DWORD
webiface_bytes_recv (void);

#endif
