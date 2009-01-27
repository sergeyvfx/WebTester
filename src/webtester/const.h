/**
 * WebTester Server - server of on-line testing system
 *
 * Constant definitions
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _wt_cont_h_
#define _wt_cont_h_

#ifndef SHARED_SUFFIX
#  define SHARED_SUFFIX ".so"
#endif

#define WEBTESTER_USER   "webtester"
#define WEBTESTER_GROUP  "webtester"

#define LIB_PREFIX "lib"
#define LIB_SUFFIX SHARED_SUFFIX

#define CONFIG_FILE "/home/webtester/conf/webtester.conf"

/****
 * CORE HOOK callbacks names
 */

#define CORE_ACTIVATE        "CORE_Activate"
#define CORE_DEACTIVATE      "CORE_Deactivate"
#define CORE_UPLOADPROBLEMS  "CORE_UploadProblems"
#define CORE_STOPTESTING     "CORE_StopTesting"

#define WEBIFACE_LIBNAME "WebInterface"

#endif
