/**
 * WebTester Server - server of on-line testing system
 *
 * Command line context processor
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _cmd_h_
#define _cmd_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/dynastruc.h>
#include <libwebtester/mutex.h>

/********
 * Type definitions
 */

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

/********
 *
 */

/* Parse command line */
int
cmd_parse_buf (const char *__buf, char ***__argv, int *__argc);

/* Free result of parsing */
void
cmd_free_arglist (char **__argv, int __argc);

/* Create new command line context */
cmd_context_t*
cmd_create_context (const char *__name);

/* Destroy command line context */
int
cmd_destroy_context (cmd_context_t *__self);

/* Register new command in context */
int
cmd_context_proc_register (cmd_context_t *__self, const char *__name,
                           cmd_entry_point entry_point);

/* Execute command in context */
int
cmd_context_execute_proc (cmd_context_t *__self, char **__argv, int __argc);

END_HEADER

#endif
