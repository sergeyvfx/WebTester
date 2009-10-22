/**
 * WebTester Server - server of on-line testing system
 *
 * Pool for hypervisor
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "hv.h"

#include <libwebtester/macrodef.h>
#include <libwebtester/core.h>
#include <libwebtester/dynastruc.h>
#include <libwebtester/conf.h>
#include <libwebtester/log.h>
#include <libwebtester/mutex.h>

#include <time.h>
#include <malloc.h>

#include <glib.h>

/* POOL storage */
static dynastruc_t *pool = NULL;

/* Size of array */
static int arr_size = RUN_HV_POOL_ARR_SIZE;

/* Lifetime of info in pool */
static int lifetime = RUN_HV_POOL_LIFETIME;

/* Delay in GetStats stuff */
static double getstats_delay = RUN_HV_POOL_GETSTATS_DELAY;

/* MAX counter of tries in GetStats stuff */
static long getstats_max_counter = RUN_HV_POOL_GETSTATS_MAX_COUNTER;

static mutex_t mutex = NULL;

/********
 * Internal built-in
 */

/**
 * Spawn new info item for pool
 *
 * @param __self - info descriptor to spawn in
 * @param __t - task stats to store on info
 */
static void
spawn_new_item (run_hvpool_item_t *__self, const struct taskstats *__t)
{
  __self->timestamp = time (0);
  __self->t = *__t;
}

/**
 * Deleter of pool element for dynastruct stuff
 *
 * @param __self - item to delete
 */
static void
pool_dyna_deleter (void *__self)
{
  run_hvpool_arr_t *arr = __self;

  if (!arr)
    {
      return;
    }

  SAFE_FREE (arr->data);
  SAFE_FREE (arr);
}

/**
 * Spawn new array to store tasks statuses
 *
 * @return new array descriptor
 */
static run_hvpool_arr_t*
spawn_new_array (void)
{
  run_hvpool_arr_t *ptr;
  ptr = malloc (sizeof (run_hvpool_arr_t));

  ptr->count = 0;
  ptr->size = arr_size;
  ptr->data = malloc (sizeof (run_hvpool_item_t) * arr_size);

  return ptr;
}

/**
 * Overview pool
 */
static void
pool_overview (void)
{
  int timestamp, i;
  run_hvpool_arr_t *arr;

  dyna_item_t *item;

  timestamp = time (0) - lifetime;

  /*
   * Mutex is already locked.
   */

  item = dyna_head (pool);

  while (item)
    {
      arr = dyna_data (item);

      /* If current array isn't fully filled, we may inerrupt overviewing */
      if (arr->count < arr->size)
        {
          return;
        }

      /*
       * TODO: We can fix this stupid cycle
       */

      /* Check for timestamps */
      for (i = 0; i < arr->count; i++)
        {
          if (arr->data[i].timestamp > timestamp)
            {
              return;
            }
        }

      dyna_delete (pool, item, pool_dyna_deleter);

      item = dyna_head (pool);
    }
}

/**
 * Read data from config file
 */
static void
read_config (void)
{
  double t = 0;

  CONFIG_INT_KEY (lifetime, "LibRUN/HyperVisor/Pool/Lifetime");
  CONFIG_INT_KEY (arr_size, "LibRUN/HyperVisor/Pool/ArraySize");

  /* Delay in GetStats stuff */
  t = 0;
  CONFIG_FLOAT_KEY (t, "LibRUN/HyperVisor/GetStatsDelay");
  if (t > 0) getstats_delay = t;

  /* Max tries in GetStats stuff */
  t = 0;
  CONFIG_FLOAT_KEY (t, "LibRUN/HyperVisor/GetStatsTimelimit");
  if (t > 0)
    {
      getstats_max_counter = t / getstats_delay;
    }

  RESET_LEZ (arr_size, RUN_HV_POOL_ARR_SIZE);
  RESET_LEZ (lifetime, RUN_HV_POOL_LIFETIME);
  RESET_LEZ (getstats_max_counter, RUN_HV_POOL_GETSTATS_MAX_COUNTER);
}

/**
 * Iterator for run_hvpool_stats_by_pid
 *
 * @param __pid - ID of process to get stats of
 * @param __stats - stats struct to store result
 * @return TRUE on success, FALSE otherwise
 */
static BOOL
run_hvpool_stats_by_pid_iter (__u32 __pid, struct taskstats *__stats)
{
  dyna_item_t *cur;
  run_hvpool_arr_t *arr;
  int i;

  mutex_lock (mutex);

  cur = dyna_head (pool);

  while (cur)
    {
      arr = dyna_data (cur);

      for (i = 0; i < arr->count; i++)
        {
          if (arr->data[i].t.ac_pid == __pid)
            {
              (*__stats) = arr->data[i].t;
              mutex_unlock (mutex);
              return TRUE;
            }
        }
      cur = dyna_next (cur);
    }
  mutex_unlock (mutex);

  return FALSE;
}

/******
 * User's backend
 */

/**
 * Initialize hypervisor pool stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
run_hvpool_init (void)
{
  read_config ();
  pool = dyna_create ();
  mutex = mutex_create ();
  return 0;
}

/**
 * Uninitialize hypervisor pool stuff
 */
void
run_hvpool_done (void)
{
  mutex_free (mutex);

  dyna_destroy (pool, pool_dyna_deleter);
}

/**
 * Put data to hypervisor pool
 *
 * @param __self - task stats to store
 */
void
run_hvpool_put (const struct taskstats *__self)
{
  run_hvpool_arr_t *tail;
  run_hvpool_item_t item;

  if (!mutex)
    {
      return;
    }

  mutex_lock (mutex);

  DEBUG_LOG ("librun", "hvpool: Put task with pid %u...\n", __self->ac_pid);

  /* Overview pool and drop out of date info */
  pool_overview ();

  spawn_new_item (&item, __self);

  tail = dyna_data (dyna_tail (pool));

#ifdef __DEBUG
  int _size = 0;
  if (tail)
    {
      _size = tail->count;
    }
  DEBUG_LOG ("librun", "hvpool: pool size: %ld\n", _size);
#endif

  /* Some space in last array */
  if (tail && tail->count < tail->size)
    {
      DEBUG_LOG ("librun", "hvpool: Creating new node for task %u...\n",
                 __self->ac_pid);
      tail->data[tail->count++] = item;
    }
  else
    {
      /* Need to spawn new array */
      run_hvpool_arr_t *arr = 0;
      DEBUG_LOG ("librun", "hvpool: Spawning new array for task %u...\n",
                 __self->ac_pid);
      arr = spawn_new_array ();
      arr->data[arr->count++] = item;
      dyna_append (pool, arr, 0);
    }

  mutex_unlock (mutex);

  DEBUG_LOG ("librun", "hvpool: Task %u added to pool\n", __self->ac_pid);

}

/**
 * Get ACCT stats info by PID of process from pool
 *
 * @param __pid - ID of process to get stats of
 * @param __stats - stats struct to store result
 * @return TRUE on success, FALSE otherwise
 */
BOOL
run_hvpool_stats_by_pid (__u32 __pid, struct taskstats *__stats)
{
  long counter = 0;
  static struct timespec timestruc;
  timestruc.tv_sec = ((unsigned long long)
          (getstats_delay * NSEC_COUNT)) / NSEC_COUNT;
  timestruc.tv_nsec = ((unsigned long long)
          (getstats_delay * NSEC_COUNT)) % NSEC_COUNT;

  DEBUG_LOG ("librun", "hvpool: Get acct status by pid %u\n", __pid);

  /* Wait for info from kernel */

  /*
   * TODO: But can we optimize this sucky cycle??
   */

  for (;;)
    {
      if (run_hvpool_stats_by_pid_iter (__pid, __stats))
        {
          DEBUG_LOG ("librun", "hvpool: Got acct status by pid %u. "
                               "hvpool: rss: %lld, time: %lld\n",
                     __pid, __stats->hiwater_rss, __stats->ac_etime);
          return TRUE;
        }

      if (counter > getstats_max_counter)
        {
          break;
        }

      nanosleep (&timestruc, 0);
      counter++;
    }

  DEBUG_LOG ("librun", "hvpool: "
                       "Failed getting acct status by pid %u\n", __pid);

  return FALSE;
}
