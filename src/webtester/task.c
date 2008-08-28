/*
 *
 * ================================================================================
 *  task.h
 * ================================================================================
 *
 *  Stuff for manipulating tasks for test
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <libwebtester/core.h>
#include <libwebtester/plugin.h>
#include <libwebtester/util.h>

#include "task.h"
#include "const.h"

#include <malloc.h>
#include <string.h>
#include <dlfcn.h>

////////////////////////////////////////
// WebIFACE back-end


#define LOADSYM(proc,type,sym) \
  { \
    PLUGIN_SYM(WEBIFACE_LIBNAME,proc,type,sym); \
    if (!proc) return -1; \
  }

// Procedures' types
typedef int (*get_task_list_proc)   (dynastruc_t *__tasklist, int __queue_size);
typedef int (*restore_task_proc)    (wt_task_t *__self);
typedef int (*get_task_proc)        (wt_task_t *__self);
typedef int (*put_solution_proc)    (wt_task_t *__self);
typedef int (*reset_status_proc)    (void);

// Procedures' entrypoints
static get_task_list_proc   get_task_list;
static restore_task_proc    restore_task;
static get_task_proc        get_task;
static put_solution_proc    put_solution;
static reset_status_proc    reset_status;

static int      // Loading symbols from WebInterface library
load_symbols                       (void)
{
  LOADSYM (get_task_list,  get_task_list_proc,  "webiface_get_task_list");
  LOADSYM (restore_task,   restore_task_proc,   "webiface_restore_task");
  LOADSYM (get_task,       get_task_proc,       "webiface_get_task");
  LOADSYM (put_solution,   put_solution_proc,   "webiface_put_soution");
  LOADSYM (reset_status,   reset_status_proc,   "webiface_reset_status");
  return 0;
}

int             // Initialize task stuff
wt_task_init                       (void)
{
  if (load_symbols ())
    return -1;
  return 0;
}

void            // Uninitialize task stuff
wt_task_done                       (void)
{
}

int             // Get list of tasks
wt_get_task_list                   (dynastruc_t *__tasklist, int __queue_size)
{
  if (!get_task_list) return -1;
  return get_task_list (__tasklist, __queue_size);
}

int             // Restore task status at WebInterface
wt_restore_task                    (wt_task_t *__self)
{
  if (!restore_task) return -1;
  return restore_task (__self);
}

int             // Get all parameters for task
wt_get_task                        (wt_task_t *__self)
{
  if (!get_task) return -1;
  return get_task (__self);
}

int             // Put tested task to WebInterface
wt_put_solution                    (wt_task_t *__self)
{
  if (!put_solution) return -1;
  return put_solution (__self);
}

int             // Put tested task to WebInterface
wt_reset_status                    (void)
{
  if (!reset_status) return -1;
  return reset_status ();
}

////////////////////////////////////////
// General stuff

wt_task_t*      // Spawn new task and preset fields
wt_spawn_new_task                  (long __solution_id, int __library_id)
{
  wt_task_t *ptr=malloc (sizeof (wt_task_t));

  memset (ptr, 0, sizeof (wt_task_t));

  ptr->sid    = __solution_id;
  ptr->lid    = __library_id;
  ptr->status = TS_PENDING;
  
  ptr->timestamp=now ();

  ptr->input_params  = assarr_create ();
  ptr->output_params = assarr_create ();
  
  ptr->buffers.lock=mutex_create ();

  TASK_FREE_BUFFER (*ptr);
  TASK_FREE_RESULT_MESSAGE (*ptr);
  TASK_FREE_LOG (*ptr);
  return ptr;
}

void            // Free task info struct
wt_task_free                       (wt_task_t *__self)
{
  assarr_destroy (__self->input_params, assarr_deleter_free_ref_data);
  assarr_destroy (__self->output_params, assarr_deleter_free_ref_data);

  mutex_free (__self->buffers.lock);

  free (__self);
}

int             // Comparator for searching for task in dynastruc storage
wt_task_search_comparator          (void *__l, void *__r)
{
  wt_task_t *l=__l, *r=__r;
  return (l->lid==r->lid  && l->sid==r->sid);
}

void
wt_task_deleter                   (void *__self)
{
  if (!__self) return;
  wt_task_free (__self);
}

void
wt_task_deleter_with_restore      (void *__self)
{
  if (!__self) return;
  wt_restore_task (__self);
  wt_task_free (__self);
}
