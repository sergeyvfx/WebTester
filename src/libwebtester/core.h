/**
 * WebTester Server - server of on-line testing system
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _core_h_
#define _core_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#define MSG_INFO     0x0001
#define MSG_WARNING  0x0002
#define MSG_ERROR    0x0004
#define MSG_DEBUG    0x0008
#define MSG_LOG      0x0010
#define MSG_NOLOG    0x0020

#define SIG_HUP        1
#define SIG_INT        2
#define SIG_QUIT       3
#define SIG_ILL        4
#define SIG_TRAP       5
#define SIG_ABRT       6
#define SIG_FPE        8
#define SIG_KILL       9
#define SIG_SEGV      11
#define SIG_PIPE      13
#define SIG_ALRM      14
#define SIG_TERM      15

#include <time.h>
#include <libwebtester/build-stamp.h>
#include <libwebtester/core-debug.h>
#include <libwebtester/dynastruc.h>

#define CMSG_OK()               core_print (MSG_INFO, " ok.\n")
#define CMSG_FAILED()           core_print (MSG_INFO, " failed!\n")
#define CMSG_FAILED_S(__err)    core_print (MSG_INFO, " failed! %s\n", __err)
#define CMSG_DONE()             core_print (MSG_INFO, " done.\n")

#define CORE_OUTPUT_LINES       256

#define CORE_PRINT_HOOK 1

/* Initialize CORE */
int
core_init (void);

/* Uninitialize CORE */
void
core_done (void);

/* Get time when CORE was started */
time_t
core_get_starttime (void);

/* Get uptime of CORE */
time_t
core_get_uptime (void);

/* Initialize version string */
void
core_init_version_string (void);

/* Get version string */
char*
core_get_version_string (void);

/* CORE has been crashed :( */
void
core_oops (char *__text, ...);

/********
 * Input/output stuff
 */

/* Set CORE silence */
void
core_set_silent (int __silent);

/* Get CORE silence */
int
core_get_silent (void);

/* Print CORE message */
void
core_print (int __type, char *__text, ...);

/* Get CORE IO buffer */
char**
core_output_buffer (int *__count);

/* Uninitialize CORE IO stuff */
void
core_io_done (void);

/********
 * Error tracking stuff
 */

/* Set last CORE error */
void
core_set_last_error (char *__text, ...);

/* Get description of las error */
char*
core_get_last_error (void);

/********
 *
 */

/* Register path for search */
void
core_register_path (char *__path);

/* Register paths from fonfig file */
void
core_register_paths_from_config (void);

/* Get first registered path */
char*
core_first_registered_path (void);

/* Get last registered path */
char*
core_last_registered_path (void);

/* Get next registered path */
char*
core_next_registered_path (void);

/* Get previous registered path */
char*
core_prev_registered_path (void);

/* Get list of registered paths */
dynastruc_t*
core_registered_paths (void);

/* Unregister registered pathes */
void
core_unregister_paths (void);

/********
 * CORE DEBUG stuff
 */

/* Enter CORE to DEBUG mode */
void
core_enter_debug_mode (void);

/* Check is CORE in debuf mode */
int
core_is_debug_mode (void);

/********
 *
 */

/* Kill process */
void
core_kill_process (int __pid, int __signal);


END_HEADER

#endif
