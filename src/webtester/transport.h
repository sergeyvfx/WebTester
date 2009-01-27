/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_TRANSPORT_H_
#define _WT_TRANSPORT_H_

#include "autoinc.h"

BEGIN_HEADER

#include <libwebtester/network-soup.h>

/* Initialize transport stuff */
int
wt_transport_init (void);

/* Uninitialize transport stuff */
void
wt_transport_done (void);

/* Prepare URL for transporting */
void
wt_transport_prepare_url (const char *__self, char *__out);

/* Send message */
http_message_t*
wt_transport_send_message (const char *__url);

END_HEADER

#endif
