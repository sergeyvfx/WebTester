/*
 *
 * ================================================================================
 *  scheduler.c
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "scheduler.h"
#include "dynastruc.h"
#include "util.h"
#include "mutex.h"

#include <malloc.h>


#define CHECK() \
    if (!mutex || !scheduler) return -1;

#define CHECK_VOID() \
    if (!mutex || !scheduler) return;


////////////////////////////////////////
//

typedef struct {
  scheduler_callback callback;
  void               *data;
  __u64_t            interval;
  timeval_t          last_call;
} scheduler_item_t;

////////////////////////////////////////
//

static dynastruc_t *scheduler = 0;
static mutex_t       mutex    = 0;

////////////////////////////////////////
//

static scheduler_item_t*
spawn_new_item                     (scheduler_callback __callback, void *__data, __u64_t __interval)
{
  scheduler_item_t *ptr;

  if (!__callback) return 0;

  ptr=malloc (sizeof (scheduler_item_t));

  if (!ptr)
    return 0;

  ptr->callback  = __callback;
  ptr->data      = __data;
  ptr->interval  = __interval;
  ptr->last_call = now ();

  return ptr;
}

////////////////////////////////////////
//

int             // Initialize scheduler stuff
scheduler_init                     (void)
{
  scheduler=dyna_create ();
  mutex=mutex_create ();
  return 0;
}

void            // Uninitialize scheduler stuff
scheduler_done                     (void)
{
  if (scheduler)
    dyna_destroy (scheduler, dyna_deleter_free_ref_data);
  if (mutex)
    mutex_free (mutex);
}

int             // Add task to scheduler
scheduler_add                      (scheduler_callback __callback, void *__data, __u64_t __interval)
{
  scheduler_item_t *item=0;

  CHECK ();  
  
  mutex_lock (mutex);
  item=spawn_new_item (__callback, __data, __interval);
  if (!item)
    {
      mutex_unlock (mutex);
      return -1;
    }
  dyna_append (scheduler, item, 0);
  mutex_unlock (mutex);
  return 0;
}

void            // Remove task from scheduler
scheduler_remove                   (scheduler_callback __callback)
{
  dyna_item_t *cur, *dummy;
  scheduler_item_t *item;

  CHECK_VOID ();

  mutex_lock (mutex);
  
  cur=dyna_head (scheduler);
  
  while (cur)
    {
      dummy=cur;
      item=dyna_data (cur);
      cur=dyna_next (cur);
      
      if (item->callback==__callback)
        dyna_delete (scheduler, dummy, dyna_deleter_free_ref_data);
    }
  
  mutex_unlock (mutex);
}

void
scheduler_overview                 (void)
{
  timeval_t cur_time;
  scheduler_item_t *item;

  CHECK_VOID ();

  mutex_lock (mutex);

  cur_time=now ();
  
  DYNA_FOREACH (scheduler, item);
    if (CHECK_TIME_DELTA (item->last_call, cur_time, item->interval))
      {
        item->callback (item->data);
        item->last_call=cur_time;
      }
  DYNA_DONE;
  
  mutex_unlock (mutex);
}
