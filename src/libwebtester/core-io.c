/*
 *
 * =============================================================================
 *  core_io.c
 * =============================================================================
 *
 *  Some core built-in stuff
 *
 *  Written (by Nazgul) under GPL
 *
*/

#include "core.h"
#include "smartinclude.h"
#include "log.h"

#include <stdio.h>

static int core_silent=0;

void
core_set_silent                    (int __silent)
{
  core_silent=__silent;
}

int
core_get_silent                    (void)
{
  return core_silent;
}

void
core_print                         (int __type, char *__text, ...)
{
  static char print_buf[65536];
  if (core_get_silent ()) return; // Silent mode
  PACK_ARGS (__text, print_buf, 65536);

  if (__type&MSG_DEBUG)
    {
#ifdef __DEBUG
      if (!core_is_debug_mode ()) return;
  
      if (!(__type&MSG_LOG))
        printf ("[DEBUG] %s", print_buf);
      if (!(__type&MSG_NOLOG))
        log_printf ("[DEBUG] %s", print_buf);
#endif
    } else {
      if (!(__type&MSG_LOG))
        printf ("%s", print_buf);
      if (!(__type&MSG_NOLOG))
        log_printf ("%s", print_buf);
  }
  fflush (stdout);
}
