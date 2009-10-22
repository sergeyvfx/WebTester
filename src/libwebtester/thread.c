/**
 * WebTester Server - server of on-line testing system
 *
 * Threading stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "thread.h"

#include <malloc.h>
#include <glib.h>

#define CS_MUTEX_APPENDIX 128

static mutex_t *cs_mutexes = NULL;
static long cs_mutexes_length = 0;
static long cs_mutexes_count = 0;

static mutex_t mutex = NULL;

/**
 * Initialize threading stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
thread_init (void)
{
  g_thread_init (0);
  if (!g_thread_supported ())
    {
      return -1;
    }

  mutex = mutex_create ();

  return 0;
}

/**
 * Uninitialize threading stuff
 */
void
thread_done (void)
{
  if (mutex)
    {
      mutex_free (mutex);
      mutex = 0;
    }
}

/**
 * Register mutex for critical section
 *
 * @param __self - mutex to register
 */
void
thread_cs_mutex_register (mutex_t __self)
{
  mutex_lock (mutex);

  if (cs_mutexes_count >= cs_mutexes_length)
    {
      mutex_t *smutex;
      int i;

      smutex = cs_mutexes;

      cs_mutexes_length += CS_MUTEX_APPENDIX;
      cs_mutexes = malloc (sizeof (mutex_t) * cs_mutexes_length);

      for (i = 0; i < cs_mutexes_count; ++i)
        cs_mutexes[i] = smutex[i];

      SAFE_FREE (smutex);
    }

  cs_mutexes[cs_mutexes_count++] = __self;

  mutex_unlock (mutex);
}
