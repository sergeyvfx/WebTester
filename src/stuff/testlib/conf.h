/**
 * WebTester Server - server of on-line testing system
 *
 * Config file for TestLib
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_TESTLIB_CONF_H_
#define _WT_TESTLIB_CONF_H_

#define MAXINT     32767
#define MAXLONGINT 2147483647

/* Allow `+` as valid setter of sign for integer values */
#define ALLOW_POSITIVE_SIGN        1
/* Allow `+` as valid setter of sign for floating-point values */
#define ALLOW_FLOAT_POSITIVE_SIGN  1

#define ENABLE_RANGE_CHECK         1

#endif
