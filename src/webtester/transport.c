/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
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

typedef http_message_t* (*webiface_send_message_proc) (const char *__url);
typedef void (*webiface_prepare_url_proc) (const char *__self, char *__out);

static webiface_prepare_url_proc prepare_url;
static webiface_send_message_proc send_message;

/**
 * Initialize transport stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
wt_transport_init (void)
{
  LOADSYM (prepare_url,  webiface_prepare_url_proc,  "webiface_prepare_url");
  LOADSYM (send_message, webiface_send_message_proc, "webiface_send_message");

  return 0;
}

/**
 * Uninitialize transport stuff
 */
void
wt_transport_done (void) {
}

/**
 * Prepare URL for transporting
 *
 * @param __self - source string
 * @param __out - destination string
 */
void
wt_transport_prepare_url (const char *__self, char *__out)
{
  if (prepare_url)
    {
      prepare_url (__self, __out);
    }
}

/**
 * Send message
 *
 * @param __url - url to send message to
 * @return message descriptor
 */
http_message_t*
wt_transport_send_message (const char *__url)
{
  if (!send_message)
    {
      return NULL;
    }

  return send_message (__url);
}
