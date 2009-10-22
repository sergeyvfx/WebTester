/**
 * WebTester Server - server of on-line testing system
 *
 * IPC interface for LibRUN
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _RUN_IPC_H_
#define _RUN_IPC_H_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/cmd.h>
#include <libwebtester/types.h>

#include <librun/run.h>

/* Default IPC port */

#define RUN_IPC_PORT         13667

/* Default IPC host to bint to */
#define RUN_IPC_HOST         "127.0.0.1"

/* Default IPC client to connect to */
#define RUN_IPC_CLIENT_HOST  "127.0.0.1"

/* MAX IPC socket  stack size */
#define RUN_SOCK_STACK_SIZE   1024

/* MAX count of clients */
#define RUN_IPC_MAX_CLIENTS   1024

/* Max timeout of keeping client registered */
#define RUN_IPC_MAX_KEEP_TIMEOUT 120.0

/****
 * Type defenitions
 */

typedef struct
{
  /* Num in array */
  int num;

  /* Accepted socked */
  int sock;

  long unique;
  char security_key[RUN_KEY_LENGTH];

  char cmd[RUN_SOCK_STACK_SIZE];

  timeval_t timestamp;
} run_ipc_client;

/*******
 *
 */

/* Initialize IPC stuff */
int
run_ipc_init (void);

/* Uninitialize IPC stff */
void
run_ipc_done (void);

/* Port of IPC stuff */
unsigned int
run_ipc_port (void);

/* Host to connect to from client */
char*
run_ipc_client_host (void);

/* Current IPC user */
run_ipc_client*
run_ipc_current_user (void);

/* Register IPC proc at context */
void
run_ipc_register_proc (const char *__name, cmd_entry_point __entry_point);

/* Answer to current client */
int
run_ipc_socket_answer (const char *__text, ...);

/* Unregister client by unique */
BOOL
run_ipc_unregister_by_unique (int __unique);

/********
 *
 */

/* Listen for incoming tconnections */
int
run_ipc_listen (void);

/* Parse commands from clients */
int
run_ipc_interact (void);

/*******
 * IPC built-in
 */

/* IPC command `unique` */
int
run_ipc_proc_unique (int __argc, char **__argv);

/* IPC command `security` */
int
run_ipc_proc_security (int __argc, char **__argv);

/* IPC command `lrvmpid` */
int
run_ipc_proc_lrvmpid (int __argc, char **__argv);

/* IPC command `taskpid` */
int
run_ipc_proc_taskpid (int __argc, char **__argv);

/* IPC command `finalize` */
int
run_ipc_proc_finalize (int __argc, char **__argv);

/* IPC command `exitcode` */
int
run_ipc_proc_exit_code (int __argc, char **__argv);

/* IPC command `termsig` */
int
run_ipc_proc_termsig (int __argc, char **__argv);

/* IPC command `stopsig` */
int
run_ipc_proc_stopsig (int __argc, char **__argv);

/* IPC command `continue` */
int
run_ipc_proc_continue (int __argc, char **__argv);

/* IPC command `base` */
int
run_ipc_proc_base (int __argc, char **__argv);

/* IPC command `exec_error` */
int
run_ipc_proc_exec_error (int __argc, char **__argv);

/* IPC command `close_pipe` */
int
run_ipc_proc_close_pipe (int __argc, char **__argv);

/* IPC command `validate` */
int
run_ipc_proc_validate (int __argc, char **__argv);

END_HEADER

#endif
