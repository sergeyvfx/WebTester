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

#ifndef _threads_h_
#define _threads_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/mutex.h>

#define CS_Begin \
  static mutex_t __cs_mutex = NULL; \
  if (!__cs_mutex) \
    { \
      __cs_mutex=mutex_create (); \
      thread_cs_mutex_register (__cs_mutex); \
    } \
  mutex_lock (__cs_mutex);

#define CS_End \
  mutex_unlock (__cs_mutex);

int
thread_init (void);

void
thread_done (void);

void
thread_cs_mutex_register (mutex_t __self);

END_HEADER

#endif
