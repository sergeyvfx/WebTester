/*
 *
 * ================================================================================
 *  thread.c - part of the WebTester Server
 * ================================================================================
 *
 *  Threading stuff
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "thread.h"

#include <malloc.h>
#include <glib.h>

/////////
//

#define CS_MUTEX_APPENDIX 128
static mutex_t *cs_mutexes  = NULL;
static long cs_mutexes_length = 0;
static long cs_mutexes_count = 0;

static mutex_t mutex=NULL;

////////
//

int
thread_init                        (void)
{
  g_thread_init (0);
  if (!g_thread_supported ())
    return -1;

  mutex=mutex_create ();

  return 0;
}

void
thread_done                        (void)
{
  if (mutex)
    {
      mutex_free (mutex);
      mutex=0;
    }
}

void
thread_cs_mutex_register           (mutex_t __self)
{
  mutex_lock (mutex);

  if (cs_mutexes_count>=cs_mutexes_length)
    {
      mutex_t *smutex;
      int i;

      smutex=cs_mutexes;

      cs_mutexes_length+=CS_MUTEX_APPENDIX;
      cs_mutexes=malloc (sizeof (mutex_t)*cs_mutexes_length);

      for (i=0; i<cs_mutexes_count; ++i)
        cs_mutexes[i]=smutex[i];

      SAFE_FREE (smutex);
    }

  cs_mutexes[cs_mutexes_count++]=__self;

  mutex_unlock (mutex);
}
