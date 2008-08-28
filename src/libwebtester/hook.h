/*
 *
 * =============================================================================
 *  hook.c
 * =============================================================================
 *
 *  Hooks stuff
 *
 *  Written (by Nazgul) under GPL
 *
*/

#ifndef _hook_h_
#define _hook_h_

#define HOOK_MAX_PRIORITY 64
#define HOOK_MAX_PROC_LEN 1024

#define HOOK_PRIORITY_LOW    1
#define HOOK_PRIORITY_NORMAL 2
#define HOOK_PRIORITY_HIGHT  (HOOK_MAX_PRIORITY - 1)


typedef int (*hook_callback_proc)   (void *__userData);

typedef struct {
  hook_callback_proc callback;
  void               *userData;
  char               proc[HOOK_MAX_PROC_LEN+1];
} hook_t;

typedef hook_t *hook_node_t;

int
hook_init                          (void);

int
hook_done                          (void);

int
hook_register                      (char *__proc, hook_callback_proc __callback, void *__userData, int _priority);

int
hook_unregister                    (char *__proc, hook_callback_proc __callback, int __priority);

int
hook_call                          (char *__proc);

int
hook_call_backward                 (char *__proc);

#endif
