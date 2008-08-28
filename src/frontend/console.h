/*
 *
 * ================================================================================
 *  console.h - part of the WebTester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _console_h_
#define _console_h_

#include "common.h"

#include <gtk/gtk.h>
#include <libwebtester/cmd.h>

#define console_log(__text,__args...) \
  log_to_widget ("console_view", "console_scroll", __text, ##__args)

#define console_cmd_line_clear() \
  console_cmd_line_set ("")

////////
// Common console stuff

int
console_init                       (void);

void
console_done                       (void);

void
log_to_widget                      (char *__console, char *__scroll, char *__text, ...);

void
console_set_socket                 (int __socket);

int
console_get_socket                 (void);

////////
// Console command stuff

int
console_cmd_init                   (void);

void
console_cmd_done                   (void);

void
console_cmd_send_specified         (char *__cmd);

void
console_cmd_send                   (void);

////////
// Console command line stuff

int
console_cmd_line_init              (void);

void
console_cmd_line_done              (void);

void
console_cmd_line_register          (char *__self);

char*
console_cmd_line_autocomplete      (void);

void
console_cmd_line_set               (char *__self);

char*
console_cmd_line_get               (void);

void
console_cmd_line_push              (char *__self);

void
console_cmd_line_prev              (void);

void
console_cmd_line_next              (void);

////////
// Builtin

BOOL
console_builtin_init               (void);

void
console_builtin_done               (void);

int
console_proc_exec                  (char *__cmd);

int
console_proc_register              (char *__procname, cmd_entry_point __entrypoint);

#endif
