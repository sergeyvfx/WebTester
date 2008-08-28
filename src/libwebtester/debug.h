/*
 *
 * =============================================================================
 *  debug.h
 * =============================================================================
 *
 *  
 *
 *  Written (by Nazgul) under GPL
 *
*/

#ifndef _debug_h_
#define _debug_h_

#include <config.h>
#include <libwebtester/build-stamp.h>
#include <libwebtester/core.h>
#include <libwebtester/debug.h>

#ifdef __DEBUG
#  include <stdio.h>
#  define DEBUG(__text, __args...) core_print (MSG_DEBUG, __text, ##__args)
#else
#  ifdef USER_DEBUG
#    include <stdio.h>
#    define DEBUG(__text, __args...) core_print (MSG_DEBUG, __text, ##__args)
#  else
#    define DEBUG(__text, __args...)
#  endif
#endif

#endif
