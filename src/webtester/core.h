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

#ifndef _WT_CORE_H_
#define _WT_CORE_H_

#include "autoinc.h"

BEGIN_HEADER

void
wt_core_panic (void);

void
wt_core_term (void);

void
wt_set_config_file (const char *__self);

END_HEADER

#endif
