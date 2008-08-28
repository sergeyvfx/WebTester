/*
 * ================================================================================
 *  acct_hv.c - part of the LibRUN
 * ================================================================================
 *
 *  Pool for hypervisor
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "hv.h"

#include <libwebtester/macrodef.h>
#include <libwebtester/core.h>
#include <libwebtester/dynastruc.h>
#include <libwebtester/conf.h>
#include <libwebtester/log.h>

#include <time.h>
#include <malloc.h>

#include <glib.h>

// POOL storage
static         dynastruc_t *pool    = NULL;
// Size of array
static int     arr_size             = RUN_HV_POOL_ARR_SIZE;
// Lifetime of info in pool
static int     lifetime             = RUN_HV_POOL_LIFETIME;
// Delay in GetStats stuff
static double  getstats_delay       = RUN_HV_POOL_GETSTATS_DELAY;
// MAX counter of tries in GetStats stuff
static long    getstats_max_counter = RUN_HV_POOL_GETSTATS_MAX_COUNTER;

static GMutex *mutex=NULL;

////////////////////////////////////////
// Internal built-in

static void     // Spawn new info item for pool
spawn_new_item                     (run_hvpool_item_t *__self, struct taskstats *__t)
{
  __self->timestamp=time (0);
  __self->t=*__t;
}

static void
pool_dyna_deleter                  (void *__self)
{
  run_hvpool_arr_t *arr=__self;

  if (!arr)
    return;

  SAFE_FREE (arr->data);
  SAFE_FREE (arr);
}

static run_hvpool_arr_t*
spawn_new_array                    (void)
{
  run_hvpool_arr_t *ptr;
  ptr=malloc (sizeof (run_hvpool_arr_t));

  if (!ptr)
    core_oops ("Memory exhausted!");

  ptr->count=0;
  ptr->size=arr_size;
  ptr->data=malloc (sizeof (run_hvpool_item_t)*arr_size);

  return ptr;
}

static void     // Overviewing of pool
pool_overview                      (void)
{
  int timestamp, i;
  run_hvpool_arr_t *arr;

  dyna_item_t *item;

  timestamp=time (0)-lifetime;

  //
  // TODO:
  //  Mutex is already locked.
  //

  item=dyna_head (pool);

  while (item)
    {
      arr=dyna_data (item);

      // If current array isn't fully filled, we may inerrupt overviewing
      if (arr->count<arr->size)
        return;

      // Check for timestamps
      for (i=0; i<arr->count; i++)
        if (arr->data[i].timestamp>timestamp)
          return;

      dyna_delete (pool, item, pool_dyna_deleter);

      item=dyna_head (pool);
    }
}

////////////////////////////////////////
//

static void     // Read data from config file
read_config                        (void)
{
  double t=0;

  CONFIG_INT_KEY   (lifetime,             "LibRUN/HyperVisor/Pool/Lifetime");
  CONFIG_INT_KEY   (arr_size,             "LibRUN/HyperVisor/Pool/ArraySize");

  // Delay in GetStats stuff
  t=0;
  CONFIG_FLOAT_KEY (t,                    "LibRUN/HyperVisor/GetStatsDelay");
  if (t>0) getstats_delay=t;

  // Max tries in GetStats stuff
  t=0;
  CONFIG_FLOAT_KEY (t, "LibRUN/HyperVisor/GetStatsTimelimit");
  if (t>0) getstats_max_counter=t/getstats_delay;

  RESET_LEZ (arr_size, RUN_HV_POOL_ARR_SIZE);
  RESET_LEZ (lifetime, RUN_HV_POOL_LIFETIME);
  RESET_LEZ (getstats_max_counter, RUN_HV_POOL_GETSTATS_MAX_COUNTER);
}

int             // Init HV pool stuff
run_hvpool_init                    (void)
{
  read_config ();
  pool=dyna_create ();
  mutex=g_mutex_new ();
  return 0;
}

void            // Uninitialize HV pool stuff
run_hvpool_done                    (void)
{
  G_FREE_LOCKED_MUTEX (mutex);

  dyna_destroy (pool, pool_dyna_deleter);
}

void            // Put data to HV pool
run_hvpool_put                     (struct taskstats *__self)
{
  run_hvpool_arr_t  *tail;
  run_hvpool_item_t item;

  if (!mutex)
    return;

  g_mutex_lock (mutex);

  DEBUG_LOG ("librun", "hvpool: Put task with pid %u...\n", __self->ac_pid);

  // Overview pool and drop out of date info
  pool_overview ();

  spawn_new_item (&item, __self);

  tail=dyna_data (dyna_tail (pool));

#ifdef __DEBUG
  int _size=0;
  if (tail) _size=tail->count;
  DEBUG_LOG ("librun", "hvpool: pool size: %ld\n", _size);
#endif

  if (tail && tail->count<tail->size) // Some space in last array
    {
      DEBUG_LOG ("librun", "hvpool: Creating new node for task %u...\n", __self->ac_pid);
      tail->data[tail->count++]=item;
    } else
    {
      // Need to spawn new array
      run_hvpool_arr_t *arr=0;
      DEBUG_LOG ("librun", "hvpool: Spawning new array for task %u...\n", __self->ac_pid);
      arr=spawn_new_array ();
      arr->data[arr->count++]=item;
      dyna_append (pool, arr, 0);
    }

  g_mutex_unlock (mutex);

  DEBUG_LOG ("librun", "hvpool: Task %u added to pool\n", __self->ac_pid);

}

static BOOL
run_hvpool_stats_by_pid_iter       (__u32 __pid, struct taskstats *__stats)
{
  dyna_item_t      *cur;
  run_hvpool_arr_t *arr;
  int              i;

  g_mutex_lock (mutex);

  cur=dyna_head (pool);

  while (cur)
    {
      arr=dyna_data (cur);

      for (i=0; i<arr->count; i++)
        {
          if (arr->data[i].t.ac_pid==__pid)
            {
              (*__stats)=arr->data[i].t;
              g_mutex_unlock (mutex);
              return TRUE;
            }
        }
      cur=dyna_next (cur);
    }
  g_mutex_unlock (mutex);

  return FALSE;
}

BOOL            // Get ACCT stats info by PID of running process
run_hvpool_stats_by_pid            (__u32 __pid, struct taskstats *__stats)
{
  long counter=0;
  static struct timespec timestruc;
  timestruc.tv_sec  = ((long)(getstats_delay*NSEC_COUNT))/NSEC_COUNT;
  timestruc.tv_nsec = ((long)(getstats_delay*NSEC_COUNT))%NSEC_COUNT;

  DEBUG_LOG ("librun", "hvpool: Get acct status by pid %u\n", __pid);

  // Wait for info from kernel

  //
  // TODO:
  //  But can we optimize this sucky cycle??
  //

  for (;;)
    {
      if (run_hvpool_stats_by_pid_iter (__pid, __stats))
        {
          DEBUG_LOG ("librun", "hvpool: Got acct status by pid %u. hvpool: rss: %lld, time: %lld\n\n", __pid, __stats->hiwater_rss, __stats->ac_etime);
          return TRUE;
        }
      if (counter>getstats_max_counter) break;
      nanosleep (&timestruc, 0);
      counter++;
    }

  DEBUG_LOG ("librun", "hvpool: Failed getting acct status by pid %u\n", __pid);

  return FALSE;
}
