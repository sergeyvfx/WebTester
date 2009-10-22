/**
 * WebTester Server - server of on-line testing system
 *
 * Uniques for LibRUN
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "unique.h"

#include <libwebtester/unique.h>
#include <glib.h>

static unique_pool_t *pool = NULL;
static GMutex *mutex = NULL;

/**
 * Initialize LibRUN unique stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
run_unique_init (void)
{
  pool = unique_pool_create ();
  mutex = g_mutex_new ();

  if (!pool)
    {
      core_set_last_error ("LibRUN: Error initializing unique pool stuff");
      return -1;
    }

  return 0;
}

/**
 * Uninitialize LibRUN unique stuff
 */
void
run_unique_done (void)
{
  if (mutex)
    {
      g_mutex_free (mutex);
    }

  unique_pool_destroy (pool);
  pool = NULL;
}

/**
 * Allocate new unique id
 */
int
run_unique_alloc (void)
{
  int res;
  g_mutex_lock (mutex);
  res = unique_alloc_uid (pool);
  g_mutex_unlock (mutex);
  return res;
}

/**
 * Release allocated unique od
 */
void
run_unique_release (int __uid)
{
  g_mutex_lock (mutex);
  unique_release_uid (pool, __uid);
  g_mutex_unlock (mutex);
}
