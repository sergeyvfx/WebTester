/**
 * WebTester Server - server of on-line testing system
 *
 * Logging stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef log_h
#define log_h

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#define MAX_LOG_SIZE   (30*1024*1024)
#define MAX_LOG_FILES  50

#define LOG_PACKER     "bz2"
#define LOG_PACKED_EXT ".bz2"

/* Initialize LOG stuff */
int
log_init (const char *__fn);

/* Uninitialize LOG stuff */
void
log_done (void);

/* Print text to LOG file */
void
log_printf (char *__text, ...);

END_HEADER

#endif
