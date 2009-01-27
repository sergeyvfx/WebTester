/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _pid_h_
#define _pid_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

int
create_pid_file (char *__fn);

int
delete_pid_file (char *__fn);

END_HEADER

#endif
