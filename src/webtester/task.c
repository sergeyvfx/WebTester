/**
 * WebTester Server - server of on-line testing system
 *
 * Stuff for manipulating tasks for test
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <libwebtester/core.h>
#include <libwebtester/plugin.h>
#include <libwebtester/util.h>

#include "task.h"
#include "const.h"
#include "belts.h"

#include <malloc.h>
#include <string.h>
#include <dlfcn.h>

/****
 * WebIFACE back-end
 */


#define LOADSYM(proc,type,sym) \
  { \
    PLUGIN_SYM(WEBIFACE_LIBNAME,proc,type,sym); \
    if (!proc) \
      { \
        return -1; \
      } \
  }

/* Procedures' types */
typedef int (*get_task_list_proc) (dynastruc_t *__tasklist, int __queue_size);
typedef int (*restore_task_proc) (wt_task_t *__self);
typedef int (*get_task_proc) (wt_task_t *__self);
typedef int (*put_solution_proc) (wt_task_t *__self);
typedef int (*reset_status_proc) (void);

/* Procedures' entrypoints */
static get_task_list_proc get_task_list;
static restore_task_proc restore_task;
static get_task_proc get_task;
static put_solution_proc put_solution;
static reset_status_proc reset_status;

/**
 * Loading symbols from WebInterface library
 *
 * @return zero on success, non-zero otherwise
 */
static int
load_symbols (void)
{
  LOADSYM (get_task_list, get_task_list_proc, "webiface_get_task_list");
  LOADSYM (restore_task, restore_task_proc, "webiface_restore_task");
  LOADSYM (get_task, get_task_proc, "webiface_get_task");
  LOADSYM (put_solution, put_solution_proc, "webiface_put_solution");
  LOADSYM (reset_status, reset_status_proc, "webiface_reset_status");
  return 0;
}

/**
 * Initialize task stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
wt_task_init (void)
{
  if (load_symbols ())
    {
      return -1;
    }

  return 0;
}

/**
 * Uninitialize task stuff
 */
void
wt_task_done (void)
{
  /*
   * Nothing to do
   */
}

/**
 * Get list of tasks
 *
 * @param __tasklist - dyna to store tasks
 * @param __queue_size - maximal size of queue
 */
int
wt_get_task_list (dynastruc_t *__tasklist, int __queue_size)
{
  if (!get_task_list)
    {
      return -1;
    }

  return get_task_list (__tasklist, __queue_size);
}

/**
 * Restore task status at WebInterface
 *
 * @param __self - task to be restored
 * @return zero on success, non-zero otherwise
 */
int
wt_restore_task (wt_task_t *__self)
{
  if (!restore_task)
    {
      return -1;
    }

  return restore_task (__self);
}

/**
 * Get all parameters for task
 *
 * @param __self - task to get all parameters for
 * @return zero on success, non-zero otherwise
 */
int
wt_get_task (wt_task_t *__self)
{
  if (!get_task)
    {
      return -1;
    }

  return get_task (__self);
}

/**
 * Put tested task to WebInterface
 *
 * @param __self - task to be put
 * @return zero on success, non-zero otherwise
 */
int
wt_put_solution (wt_task_t *__self)
{
  if (!put_solution)
    {
      return -1;
    }

  return put_solution (__self);
}

/**
 * Put tested task to WebInterface
 *
 * @reset status of all untested tasks
 * @return zero on success, non-zero otherwise
 */
int
wt_reset_status (void)
{
  if (!reset_status)
    {
      return -1;
    }

  return reset_status ();
}

/****
 * General stuff
 */

/**
 * Spawn new task and preset fields
 *
 * @param __solution_id - ID of solution
 * @param __library_id - id of library to use
 * @return new task descriptor
 */
wt_task_t*
wt_spawn_new_task (long __solution_id, int __library_id)
{
  wt_task_t *ptr = malloc (sizeof (wt_task_t));

  memset (ptr, 0, sizeof (wt_task_t));

  ptr->sid = __solution_id;
  ptr->lid = __library_id;
  ptr->status = TS_PENDING;

  ptr->timestamp = now ();

  ptr->input_params = assarr_create ();
  ptr->output_params = assarr_create ();

  ptr->buffers.lock = mutex_create ();

  ptr->chain_testing.last = 0;

  TASK_FREE_BUFFER (*ptr);
  TASK_FREE_RESULT_MESSAGE (*ptr);
  TASK_FREE_LOG (*ptr);

  return ptr;
}

/**
 * Free task info struct
 *
 * @param __self - task to be freed
 */
void
wt_task_free (wt_task_t *__self)
{
  assarr_destroy (__self->input_params, assarr_deleter_free_ref_data);
  assarr_destroy (__self->output_params, assarr_deleter_free_ref_data);

  mutex_free (__self->buffers.lock);

  free (__self);
}

/**
 * Set belts status changed
 */
void
wt_task_set_belts_status_changed (void)
{
  wt_belts_status_changed ();
}

/**
 * Comparator for searching for task in dynastruc storage
 *
 * @param __l - task stored on dyna
 * @param __r - task to compare with
 */
int
wt_task_search_comparator (void *__l, void *__r)
{
  wt_task_t *l = __l, *r = __r;
  return (l->lid == r->lid && l->sid == r->sid);
}

/**
 * Dyna task deleter
 *
 * @param __self - task to be deleted
 */
void
wt_task_deleter (void *__self)
{
  if (!__self)
    {
      return;
    }

  wt_task_free (__self);
}

/**
 * Dyna task deleter with restoring in WebIFACE
 *
 * @param __self - task to be deleted
 */
void
wt_task_deleter_with_restore (void *__self)
{
  if (!__self)
    {
      return;
    }

  wt_restore_task (__self);
  wt_task_free (__self);
}

/**
 * Add library to chain
 *
 * @param __self - task for chain testing
 * @param __lid - ID of library to test task on
 * @return zero on success, non-zero otherwise
 */
int
wt_task_chaintest (wt_task_t *__self, int __lid)
{
  int i;
  BOOL found = FALSE;

  if (!__self)
    {
      return -1;
    }

  /* Check if task has been already tested on specified library */
  for (i = 0; i < __self->chain_testing.last; ++i)
    {
      if (__self->chain_testing.lid[i] == __lid)
        {
          found = TRUE;
          break;
        }
    }

  if (found || __self->lid == __lid)
    {
      LOG ("task", "Try to send task %ld for chain testing on module %d, "
                   "which has been already added to chain.\n",
           __self->sid, __lid);
      return -1;
    }

  __self->chain_testing.lid[__self->chain_testing.last++] = __lid;
  DEBUG_LOG ("task", "Task %d send to be chain tested on module %d\n",
             __self->sid, __lid);

  return 0;
}
