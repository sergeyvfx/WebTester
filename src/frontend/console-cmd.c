/*
 *
 * ================================================================================
 *  console-cmd.c - part of the Webtester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "console.h"
#include "builtin.h"

#include <string.h>

////
//

int
console_cmd_init                   (void)
{
  console_cmd_line_init ();
  return 0;
}

void
console_cmd_done                   (void)
{
  console_cmd_line_done ();
}

void
console_cmd_send_specified         (char *__cmd)
{
  console_cmd_line_push (__cmd);
  console_log (">> %s\n", __cmd);
  if (console_proc_exec (__cmd))
    console_log ("Unknown command %s\n", __cmd);
  console_cmd_line_set ("");
}

void
console_cmd_send                   (void)
{
  char *cmd=console_cmd_line_get ();
  console_cmd_send_specified (cmd);
}
