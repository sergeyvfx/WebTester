/**
 * WebTester Server - server of on-line testing system
 *
 * HTTP network stuff abstraction
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _wt_soup_h_
#define _wt_soup_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <glib.h>
#include <libsoup/soup.h>

#define HTTP_OK 200

/****
 * Macroses
 */

#define HTTP_RESPONSE_BODY(msg)    http_message_get_response_body (msg)
#define HTTP_RESPONSE_LENGTH(msg)  http_message_get_response_length (msg)

#define HTTP_STATUS(msg)           (msg).status_code
#define HTTP_STATUS_OK(msg)        (HTTP_STATUS (msg)==HTTP_OK)

typedef SoupSession http_session_t;
typedef SoupMessage http_message_t;

/* Initialize HTTP stuff */
int
http_init (void);

/* Uninitialize HTTP stuff */
void
http_done (void);

/* Start new HTTP session */
http_session_t*
http_session_new (int __async);

/* Estended start new HTTP session */
http_session_t*
http_session_new_extended (int __async, const char *__proxy,
                           const char *__ca_file);

/* Close HTTP session */
void
http_session_free (http_session_t *__self);

/* Send message through HTTP session */
int
http_session_send_message (http_session_t *__session,
                           http_message_t *__message);

/* Prepare HTTP message */
http_message_t*
http_message_prepare (const char *__method, const char *__url);

/* Free HTTP message */
void
http_message_free (http_message_t *__self);

/* Get HTTP message response body */
const char*
http_message_get_response_body (const http_message_t *__self);

/* Get HTTP message response body length */
int
http_message_get_response_length (const http_message_t *__self);

/* Get HTTP error description */
void
http_get_error (const http_message_t *__msg, char *__out);

/* Encode URL */
void
urlencode (const char *__url, char *__out);

END_HEADER

#endif
