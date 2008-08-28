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

#include "hook.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

static int core_silent=0;

static char *lines[CORE_OUTPUT_LINES]={0};
static int lines_count=0;

////////
// Internal stuff

static void
push_line                          (char *__text, ...)
{
  int uk=0;
  static char print_buf[65536];
  PACK_ARGS (__text, print_buf, 65536);

  if (lines_count==CORE_OUTPUT_LINES-1)
    {
      int i;
      free (lines[0]);
      for (i=0; i<lines_count-1; ++i)
        lines[i]=lines[i+1];
      uk=lines_count;
    } else
      uk=lines_count++;

  lines[uk]=strdup (print_buf);
}

////////
//

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
  int silent=core_get_silent ();
  static char print_buf[65536];
  PACK_ARGS (__text, print_buf, 65536);

  if (__type&MSG_DEBUG)
    {
#if defined(__DEBUG) || defined (USER_DEBUG)
      if (!core_is_debug_mode ()) return;

      char dummy[65536];
      sprintf (dummy, "[DEBUG] %s", print_buf);
  
      if (!silent && !(__type&MSG_LOG))
        {
          printf ("%s", dummy);
          push_line ("%s", dummy);
#ifdef CORE_PRINT_HOOK
          hook_call ("CORE.Print", dummy);
#endif
        }
      if (!(__type&MSG_NOLOG))
        log_printf ("%s", dummy);
#endif
    } else {
      if (!silent && !(__type&MSG_LOG))
        {
          printf ("%s", print_buf);
          push_line ("%s", print_buf);
#ifdef CORE_PRINT_HOOK
          hook_call ("CORE.Print", print_buf);
#endif
        }
      if (!(__type&MSG_NOLOG))
        log_printf ("%s", print_buf);
  }
  fflush (stdout);
}

void
core_io_done                       (void)
{
  int i;
  for (i=0; i<lines_count; i++)
    free (lines[i]);
  lines_count=0;
}

char**
core_output_buffer                 (int *__count)
{
  if (__count)
    *__count=lines_count;
  return lines;
}
