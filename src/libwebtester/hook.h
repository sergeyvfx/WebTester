/**
 * WebTester Server - server of on-line testing system
 *
 * Hooks' stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _hook_h_
#define _hook_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#define HOOK_MAX_PRIORITY 64
#define HOOK_MAX_PROC_LEN 1024

#define HOOK_PRIORITY_LOW    1
#define HOOK_PRIORITY_NORMAL 2
#define HOOK_PRIORITY_HIGHT  (HOOK_MAX_PRIORITY - 1)

/********
 * Type definitions
 */

typedef int (*hook_callback_proc) (void *__userData, void *__callData);

typedef struct
{
  hook_callback_proc callback;
  void *userData;
  char proc[HOOK_MAX_PROC_LEN + 1];
} hook_t;

typedef hook_t *hook_node_t;

/********
 *
 */

/* Initialize hooks' stuff */
int
hook_init (void);

/* Uninitialize hooks' stuff */
void
hook_done (void);

/* Register hook */
int
hook_register (char *__proc, hook_callback_proc __callback,
               void *__userData, int _priority);

/* Unregister hook */
int
hook_unregister (char *__proc, hook_callback_proc __callback, int __priority);

/* Call hook */
int
hook_call (char *__proc, void *__data);

/* Call hook in backward mode */
int
hook_call_backward (char *__proc, void *__data);

END_HEADER

#endif
