/*
 *
 * ================================================================================
 *  cmd.h
 * ================================================================================
 *
 *  Command line context processor
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _cmd_h_
#define _cmd_h_

#include <libwebtester/dynastruc.h>
#include <libwebtester/mutex.h>

typedef int (*cmd_entry_point) (int argc, char **argv);

typedef struct
{
  char *name;
  cmd_entry_point entry_point;
} cmd_proc_t;

typedef struct
{
  char *name;
  mutex_t mutex;
  dynastruc_t *proclist;
} cmd_context_t;

int
cmd_parse_buf                      (char *__buf, char ***__argv, int *__argc);

char*
cmd_parser_iterator                (char *__self, char **__token, int *__line);

void
cmd_free_arglist                   (char **__argv, int __argc);

cmd_context_t*
cmd_create_context                 (char *__name);

int
cmd_destroy_context                (cmd_context_t *__self);

int
cmd_context_proc_register          (cmd_context_t *__self, char *__name, cmd_entry_point entry_point);

int
cmd_context_execute_proc           (cmd_context_t *__self, char **__argv, int __argc);

#endif
