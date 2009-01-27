/**
 * WebTester Server - server of on-line testing system
 *
 * Scheduling
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
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

typedef struct
{
  scheduler_callback callback;
  void *data;
  __u64_t interval;
  timeval_t last_call;
} scheduler_item_t;

static dynastruc_t *scheduler = 0;
static mutex_t mutex = 0;

/**
 * Create new task descriptor
 *
 * @param __callback - callback of task
 * @oaram __data - data associated with task
 * @param __interval - intyerval of calling
 * @return new task descriptor
 */
static scheduler_item_t*
spawn_new_item (scheduler_callback __callback, void *__data,
                __u64_t __interval)
{
  scheduler_item_t *ptr;

  if (!__callback)
    {
      return 0;
    }

  ptr = malloc (sizeof (scheduler_item_t));

  if (!ptr)
    {
      return 0;
    }

  ptr->callback = __callback;
  ptr->data = __data;
  ptr->interval = __interval;
  ptr->last_call = now ();

  return ptr;
}

/********
 * User's backend
 */

/**
 * Initialize scheduler stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
scheduler_init (void)
{
  scheduler = dyna_create ();
  mutex = mutex_create ();
  return 0;
}

/**
 * Uninitialize scheduler stuff
 */
void
scheduler_done (void)
{
  if (scheduler)
    {
      dyna_destroy (scheduler, dyna_deleter_free_ref_data);
    }

  if (mutex)
    {
      mutex_free (mutex);
    }
}

/**
 * Add task to scheduler
 *
 * @param __callback - callback of task
 * @oaram __data - data associated with task
 * @param __interval - intyerval of calling
 * @return zero on success, non-zero otherwise
 */
int
scheduler_add (scheduler_callback __callback, void *__data,
               __u64_t __interval)
{
  scheduler_item_t *item = 0;

  CHECK ();

  mutex_lock (mutex);
  item = spawn_new_item (__callback, __data, __interval);
  if (!item)
    {
      mutex_unlock (mutex);
      return -1;
    }

  dyna_append (scheduler, item, 0);
  mutex_unlock (mutex);

  return 0;
}

/**
 * Remove task from scheduler
 *
 * @param __callback - callback of task to remove
 */
void
scheduler_remove (scheduler_callback __callback)
{
  dyna_item_t *cur, *dummy;
  scheduler_item_t *item;

  CHECK_VOID ();

  mutex_lock (mutex);

  cur = dyna_head (scheduler);

  while (cur)
    {
      dummy = cur;
      item = dyna_data (cur);
      cur = dyna_next (cur);

      if (item->callback == __callback)
        {
          dyna_delete (scheduler, dummy, dyna_deleter_free_ref_data);
        }
    }

  mutex_unlock (mutex);
}

/**
 * Ovwerview tasks in scheduler
 */
void
scheduler_overview (void)
{
  timeval_t cur_time;
  scheduler_item_t *item;

  CHECK_VOID ();

  mutex_lock (mutex);

  cur_time = now ();

  DYNA_FOREACH (scheduler, item);
  if (CHECK_TIME_DELTA (item->last_call, cur_time, item->interval))
    {
      item->callback (item->data);
      item->last_call = cur_time;
    }
  DYNA_DONE;

  mutex_unlock (mutex);
}
