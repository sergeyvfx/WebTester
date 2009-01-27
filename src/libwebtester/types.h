/**
 * WebTester Server - server of on-line testing system
 *
 * Type definitions
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _wt_types_h_
#define _wt_types_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <sys/types.h>

#  ifndef BOOL

#  define BOOL int

#  ifndef FALSE
#    define FALSE 0
#  endif

#  ifndef TRUE
#    define TRUE 1
#  endif

#  endif

#ifndef DWORD
#  define DWORD unsigned long long
#endif

typedef unsigned char __u8_t;
typedef unsigned int __u16_t;
typedef unsigned long __u32_t;
typedef unsigned long long __u64_t;

typedef struct timeval timeval_t;

END_HEADER

#endif
