/**
 * WebTester Server - server of on-line testing system
 *
 * IPC core stuff module
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _ipc_h_
#define _ipc_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#define IPC_MAX_CLIENT_COUNT 1024

#include <libwebtester/cmd.h>
#include <libwebtester/sock.h>
#include <libwebtester/util.h>

#include <time.h>

typedef struct
{
  int id;
  int sock;

  char *cmd;

  int authontificated;
  char login[32];
  int access;

  unsigned long flags;

  time_t timestamp;
  char uid[64];

  char ip[32];

  timeval_t frozen_timestamp;
  int freeze_duration;
} ipc_client_t;

#define IPCCF_FROZEN 0x0001

#define IPC_PROC_ANSWER(__text,__args...) \
  sock_answer (ipc_get_current_client ()->sock, __text, ##__args)

#define IPC_PROC_ACCESS \
  (ipc_get_current_client ()->access)

#define IPC_ADMIN_REQUIRED \
  if (IPC_PROC_ACCESS<7) { \
    IPC_PROC_ANSWER ("-ERR You must have administrator previleges " \
                     "to run this command\n"); \
    return 0; \
  }

typedef int (*ipc_printf_proc_t) (const char *__text, ...);


/* Initialize IPC stuff */
int
ipc_init (const char *__host, unsigned int __port);

/* Uninitialize IPC stuff */
int
ipc_done (void);

/* Listen for incoming connections */
int
ipc_listen (void);

/* Execute commands from clients */
int
ipc_interact (void);

/* Disconnect client from server */
int
ipc_disconnect_client (ipc_client_t *__self, int __send_info);

/* Get client by it's ID */
ipc_client_t*
ipc_get_client_by_id (int __id);

/* Get currently executing client */
ipc_client_t*
ipc_get_current_client (void);

/* Is IPC client forzen? */
int
ipc_client_frozen (ipc_client_t *__self);

/* Freeze IPC client */
void
ipc_client_freeze (ipc_client_t *__self, int __duration);

/****
 * IPC procs' stuff
 */

/* Register procedure in IPC command context */
int
ipc_proc_register (const char *__procname, cmd_entry_point __entrypoint);

/* Unregister procedure from IPC command context */
int
ipc_proc_unregister (const char *__procname);

/* Handler of command `exit` */
int
ipc_proc_exit (int __argc, char **__argv);

/****
 * Blacklisting
 */

/* Initialzie IPC blacklist stuff */
int
ipc_blacklist_init (const char *__blacklist_file, long __reset_timeout);

/* Uninitialize IPC blacklist stuff */
void
ipc_blacklist_done (void);

/* Check is IP address blacklisted */
int
ipc_blacklisted (const char *__ip);

/* Blacklist IP */
void
ipc_blacklist_ip (const char *__ip, time_t __time);

/* Remove IP address from blacklist */
void
ipc_unblacklist_ip (const char *__ip);

END_HEADER

#endif
