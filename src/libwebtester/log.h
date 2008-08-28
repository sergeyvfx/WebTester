/*
 *
 * ================================================================================
 *  log.h - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _log_h_
#define _log_h_

#define MAX_LOG_SIZE   (30*1024*1024)
#define MAX_LOG_FILES  50

#define LOG_PACKER     "bz2"
#define LOG_PACKED_EXT ".bz2"

int
log_init                           (const char *__fn);

void
log_done                           (void);

void
log_printf                         (char *__text, ...);

#endif
