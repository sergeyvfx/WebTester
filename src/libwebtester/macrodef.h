/*
 *
 * =============================================================================
 *  macrodef.h
 * =============================================================================
 *
 *  Deifferent MACRO defenitions
 *
 *  Written (by Nazgul) under GPL
 *
*/

#ifndef _macrodef_h_
#define _macrodef_h_

#include <libwebtester/core-debug.h>
#include <libwebtester/log.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define SAFE_FREE(a) \
  if (a) { free (a); a=0; }
  
#ifndef MIN
#  define MIN(a,b) \
  ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#  define MAX(a,b) \
  ((a)>(b)?(a):(b))
#endif

#define PACK_ARGS(__text,__buf,__size) \
  va_list ap;\
  strcpy (__buf, "");\
  va_start (ap, __text);\
  vsnprintf (__buf, __size, __text, ap); \
  va_end (ap);

#define SET_ERROR(__text,__args...) \
  { \
    if (__error) \
      sprintf (__error, __text, ##__args); \
  }


#define USEC_COUNT (1000*1000)
#define NSEC_COUNT (1000*1000*1000)

#define RESET_LE(__self,__b,__newval) \
  if (__self<=__b) __self=__newval

#define RESET_LEZ(__self,__newval) \
  RESET_LE (__self, 0, __newval)

////
//

#define ITOL(a) (0x00000000L+a)

////
//

#define CHECK_TIME_DELTA(__self, __timestamp, __delta) \
  (tv_usec_cmp (timedist (__self, __timestamp), __delta)>0)

////////
//

#define _INFO(__text,__args...)      core_print (MSG_INFO, __text, ##__args)
#define _ERROR(__text,__args...)     core_print (MSG_ERROR, __text, ##__args)
#define _WARNING(__text,__args...)   core_print (MSG_WARNING, __text, ##__args)

#define STAT_PERMS(__m) \
  ((__m).st_mode&00777)

#define LOG(__module, __text, __args...) \
  log_printf (__module ": " __text, ##__args);

#ifdef __DEBUG
#  define DEBUG_LOG(__module, __text, __args...) \
  log_printf ("[DEBUG] "  __module ": " __text, ##__args)
#else
#  define DEBUG_LOG(__module, __text, __args...)
#endif

#define G_SAFE_FREE_MUTEX(__self)\
  if (__self) { g_mutex_free (__self); __self=0; }

#define G_FREE_LOCKED_MUTEX(__self)\
  if (__self) \
    { \
      g_mutex_lock (__self); \
      g_mutex_unlock (__self); \
      g_mutex_free (__self); \
      __self=0; \
    }

#endif
