/**
 * WebTester Server - server of on-line testing system
 *
 * Mutexes
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _mutex_h_
#define _mutex_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <glib.h>

typedef GMutex* mutex_t;

#define mutex_create()         g_mutex_new()
#define mutex_lock(__self)     g_mutex_lock(__self)
#define mutex_unlock(__self)   g_mutex_unlock(__self)
#define mutex_trylock(__self)  g_mutex_trylock(__self)
#define mutex_free(__self) \
  { \
    if (__self) \
      { \
        mutex_lock (__self); \
        mutex_unlock (__self); g_mutex_free(__self); \
      } \
  }

END_HEADER

#endif
