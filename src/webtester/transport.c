/*
 *
 * ================================================================================
 *  transport.h - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <libwebtester/plugin.h>
#include <libwebtester/core.h>

#include "const.h"
#include "transport.h"

#include <string.h>
#include <dlfcn.h>

#define LOADSYM(proc,type,sym) \
  { \
    PLUGIN_SYM(WEBIFACE_LIBNAME,proc,type,sym); \
    if (!proc) return -1; \
  }

typedef http_message_t* (*webiface_send_message_proc)   (char *__url);
typedef void            (*webiface_prepare_url_proc)    (char *__self, char *__out);

static webiface_prepare_url_proc  prepare_url;
static webiface_send_message_proc send_message;

int
wt_transport_init                  (void)
{
  LOADSYM (prepare_url,   webiface_prepare_url_proc,   "webiface_prepare_url");
  LOADSYM (send_message,  webiface_send_message_proc,  "webiface_send_message");

  return 0;
}

void
wt_transport_done                  (void)
{

}

////////
//

void
wt_transport_prepare_url           (char *__self, char *__out)
{
  if (prepare_url)
    prepare_url (__self, __out);
}

http_message_t*
wt_transport_send_message          (char *__url)
{
  if (!send_message) return 0;
  return send_message (__url);
}
