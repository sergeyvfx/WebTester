/*
 * ================================================================================
 *  unique.h - part of the LibRUN
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "unique.h"

#include <libwebtester/unique.h>
#include <glib.h>

static unique_pool_t *pool  = NULL;
static GMutex        *mutex = NULL;

int
run_unique_init                    (void)
{
  pool=unique_pool_create ();
  mutex=g_mutex_new ();
  if (!pool)
    {
      core_set_last_error ("LibRUN: Error initializing unique pool stuff");
      return -1;
    }
  return 0;
}

void
run_unique_done                    (void)
{
  if (mutex)
    {
      g_mutex_free (mutex);
    }
  unique_pool_destroy (pool);
  pool=0;
}

int
run_unique_alloc                   (void)
{
  int res;
  g_mutex_lock (mutex);
  res=unique_alloc_uid (pool);
  g_mutex_unlock (mutex);
  return res;
}

void
run_unique_release                 (int __uid)
{
  g_mutex_lock (mutex);
  unique_release_uid (pool, __uid);
  g_mutex_unlock (mutex);
}
