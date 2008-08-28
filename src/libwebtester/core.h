/*
 *
 * =============================================================================
 *  core.h
 * =============================================================================
 *
 *  Some core built-in stuff
 *
 *  Written (by Nazgul) under GPL
 *
*/

#ifndef _core_h_
#define _core_h_

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

int
core_init                          (void);

void
core_done                          (void);

time_t
core_get_starttime                 (void);

time_t
core_get_uptime                    (void);

void
core_init_version_string           (void);

char*
core_get_version_string            (void);

void
core_oops                          (char *__text, ...);

////////////////////////////////////////
// Input/output stuff

void
core_set_silent                    (int __silent);

int
core_get_silent                    (void);

void
core_print                         (int __type, char *__text, ...);

///////////////////////////////////
// Error tracking stuff
void
core_set_last_error                (char *__text, ...);

char*
core_get_last_error                (void);

////////////////////////////////////////
//

void
core_register_path                 (char *__path);

void
core_register_paths_from_config    (void);

char*
core_first_registered_path         (void);

char*
core_last_registered_path          (void);

char*
core_next_registered_path          (void);

char*
core_prev_registered_path          (void);

void
core_unregister_paths              (void);

dynastruc_t*
core_registered_paths             (void);

///////////////////////////////////////
// CORE DEBUG stuff

void
core_enter_debug_mode              (void);

int
core_is_debug_mode                 (void);

///////////////////////////////////
//

void
core_kill_process                  (int __pid, int __signal);

#endif
