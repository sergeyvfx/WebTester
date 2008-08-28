/*
 *
 * ================================================================================
 *  network-soup.h
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_soup_h_
#define _wt_soup_h_

#include <glib.h>
#include <libsoup/soup.h>

////
// 

#define HTTP_OK 200

////
// Macroses

#define HTTP_RESPONSE_BODY(msg)    http_message_get_response_body (msg)
#define HTTP_RESPONSE_LENGTH(msg)  http_message_get_response_length (msg)

#define HTTP_STATUS(msg)           (msg).status_code
#define HTTP_STATUS_OK(msg)        (HTTP_STATUS (msg)==HTTP_OK)

typedef SoupSession http_session_t;
typedef SoupMessage http_message_t;

int
http_init                          (void);

void
http_done                          (void);

http_session_t*
http_session_new                   (int __async);

http_session_t*
http_session_new_extended          (int __async, char *__proxy, char *__ca_file);

void
http_session_free                  (http_session_t *__self);

int
http_session_send_message          (http_session_t *__session, http_message_t *__message);

http_message_t*
http_message_prepare               (char *__method, char *__url);

void
http_message_free                  (http_message_t *__self);

const char*
http_message_get_response_body     (http_message_t *__self);

int
http_message_get_response_length   (http_message_t *__self);

void
http_get_error                     (http_message_t *__msg, char *__out);

void
urlencode                          (char *__url, char *__out);

#endif
