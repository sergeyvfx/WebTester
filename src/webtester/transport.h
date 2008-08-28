/*
 *
 * ================================================================================
 *  transport.h - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_transport_h_
#define _wt_transport_h_

#include <libwebtester/network-soup.h>

int
wt_transport_init                  (void);

void
wt_transport_done                  (void);

////////
//

void
wt_transport_prepare_url           (char *__self, char *__out);

http_message_t*
wt_transport_send_message          (char *__url);

#endif
