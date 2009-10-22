/**
 * WebTester Server - server of on-line testing system
 *
 * Smart including of headers
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _SMART_INCLUDE_H_
#define _SMART_INCLUDE_H_

/*
  This macroses are used in header files to ensure that the declarations
  within are properly encapsulated in an `extern "C" { .. }` block when
  included from a  C++ compiler.
 */
#ifdef __cplusplus
#  define BEGIN_HEADER extern "C" {
#  define END_HEADER }
#else
#  define BEGIN_HEADER
#  define END_HEADER
#endif

BEGIN_HEADER

/* Attribute for unused parameter to avoid */
/* compilator's waringns */
#ifdef HAVE__ATTRIBUTE__
#  define ATTR_UNUSED __attribute__((unused))
#else
#  define ATTR_UNUSED
#endif

#ifndef NO_CONFIG
#  include <config.h>
#endif

#include <libwebtester/build-stamp.h>
#include <libwebtester/package.h>
#include <libwebtester/version.h>
#include <libwebtester/errors.h>
#include <libwebtester/const.h>

#include <libwebtester/types.h>
#include <libwebtester/macrodef.h>

 #include <libwebtester/core.h>

END_HEADER

#endif
