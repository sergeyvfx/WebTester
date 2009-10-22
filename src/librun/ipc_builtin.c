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

#include "run.h"
#include "ipc.h"

#include <libwebtester/sock.h>

#include <stdlib.h>
#include <string.h>

#define ERR(__err) \
  {  \
    run_ipc_socket_answer ("-ERR %s\n", __err); \
    return -1; \
  }

#define OK \
  {  \
    run_ipc_socket_answer ("+OK\n"); \
    return 0; \
  }

#define S_OK \
  run_ipc_socket_answer ("+OK");

#define VALIDATE_PROC(proc) \
  if (!proc) ERR ("No such process"); \
  if (strcmp (proc->security_key, client->security_key)) ERR ("Security mostmatch");

/**
 * IPC command `unique`
 *
 * @param __argc - count of arguments
 * @param __argv - arguments' values
 * @return zero on success, non-zero otherwise
 */
int
run_ipc_proc_unique (int __argc, char **__argv)
{
  if (__argc != 2)
    {
      ERR ("Invalid arglist");
    }

  run_ipc_client *client = run_ipc_current_user ();
  client->unique = atoi (__argv[1]);

  OK;
}

/**
 * IPC command `security`
 *
 * @param __argc - count of arguments
 * @param __argv - arguments' values
 * @return zero on success, non-zero otherwise
 */
int
run_ipc_proc_security (int __argc, char **__argv)
{
  if (__argc != 2)
    {
      ERR ("Invalid arglist");
    }

  run_ipc_client *client = run_ipc_current_user ();
  strncpy (client->security_key, __argv[1], RUN_KEY_LENGTH - 1);

  OK;
}

/**
 * IPC command `taskpid`
 *
 * @param __argc - count of arguments
 * @param __argv - arguments' values
 * @return zero on success, non-zero otherwise
 */
int
run_ipc_proc_taskpid (int __argc, char **__argv)
{
  if (__argc != 2)
    {
      ERR ("Invalid arglist");
    }

  run_ipc_client *client = run_ipc_current_user ();
  run_process_info_t *proc = run_process_info_by_unique (client->unique);

  VALIDATE_PROC (proc);

  proc->task_pid = atol (__argv[1]);

  OK;
}

/**
 * IPC command `finalize`
 *
 * @param __argc - count of arguments
 * @param __argv - arguments' values
 * @return zero on success, non-zero otherwise
 */
int
run_ipc_proc_finalize (int __argc, char **__argv)
{
  if (__argc != 1)
    {
      ERR ("Invalid arglist");
    }

  run_ipc_client *client = run_ipc_current_user ();
  run_process_info_t *proc = run_process_info_by_unique (client->unique);

  VALIDATE_PROC (proc);

  run_finalize_executing (proc);

  OK;
}

/**
 * IPC command `exit_code`
 *
 * @param __argc - count of arguments
 * @param __argv - arguments' values
 * @return zero on success, non-zero otherwise
 */
int
run_ipc_proc_exit_code (int __argc, char **__argv)
{
  if (__argc != 2)
    {
      ERR ("Invalid arglist");
    }

  run_ipc_client *client = run_ipc_current_user ();
  run_process_info_t *proc = run_process_info_by_unique (client->unique);

  VALIDATE_PROC (proc);

  proc->exit_code = atoi (__argv[1]);

  OK;
}

/**
 * IPC command `termsig`
 *
 * @param __argc - count of arguments
 * @param __argv - arguments' values
 * @return zero on success, non-zero otherwise
 */
int
run_ipc_proc_termsig (int __argc, char **__argv)
{
  if (__argc != 2)
    {
      ERR ("Invalid arglist");
    }

  run_ipc_client *client = run_ipc_current_user ();
  run_process_info_t *proc = run_process_info_by_unique (client->unique);

  VALIDATE_PROC (proc);

  run_process_terminated (proc, atoi (__argv[1]));

  OK;
}

/**
 * IPC command `stopsig`
 *
 * @param __argc - count of arguments
 * @param __argv - arguments' values
 * @return zero on success, non-zero otherwise
 */
int
run_ipc_proc_stopsig (int __argc, char **__argv)
{
  if (__argc != 2)
    {
      ERR ("Invalid arglist");
    }

  run_ipc_client *client = run_ipc_current_user ();
  run_process_info_t *proc = run_process_info_by_unique (client->unique);

  VALIDATE_PROC (proc);

  run_process_stopped (proc, atoi (__argv[1]));

  OK;
}

/**
 * IPC command `continue`
 *
 * @param __argc - count of arguments
 * @param __argv - arguments' values
 * @return zero on success, non-zero otherwise
 */
int
run_ipc_proc_continue (int __argc, char **__argv)
{
  if (__argc != 1)
    {
      ERR ("Invalid arglist");
    }

  run_ipc_client *client = run_ipc_current_user ();
  run_process_info_t *proc = run_process_info_by_unique (client->unique);

  VALIDATE_PROC (proc);

  run_process_continued (proc);

  OK;
}

/**
 * IPC command `base`
 *
 * @param __argc - count of arguments
 * @param __argv - arguments' values
 * @return zero on success, non-zero otherwise
 */
int
run_ipc_proc_base (int __argc, char **__argv)
{
  if (__argc != 1)
    {
      ERR ("Invalid arglist");
    }

  run_ipc_client *client = run_ipc_current_user ();
  run_process_info_t *proc = run_process_info_by_unique (client->unique);

  VALIDATE_PROC (proc);

  run_process_set_base (proc);

  OK;
}

/**
 * IPC command `exec_error`
 *
 * @param __argc - count of arguments
 * @param __argv - arguments' values
 * @return zero on success, non-zero otherwise
 */
int
run_ipc_proc_exec_error (int __argc, char **__argv)
{
  if (__argc > 2)
    {
      ERR ("Invalid arglist");
    }

  run_ipc_client *client = run_ipc_current_user ();
  run_process_info_t *proc = run_process_info_by_unique (client->unique);

  VALIDATE_PROC (proc);

  if (__argc == 2)
    {
      strcpy (proc->err_desc, __argv[1]);
    }

  proc->state = PS_EXECERROR;

  OK;
}

/**
 * IPC command `validate`
 *
 * @param __argc - count of arguments
 * @param __argv - arguments' values
 * @return zero on success, non-zero otherwise
 */
int
run_ipc_proc_validate (int __argc, char **__argv)
{
  if (__argc > 1)
    {
      ERR ("Invalid arglist");
    }

  run_ipc_client *client = run_ipc_current_user ();
  run_process_info_t *proc = run_process_info_by_unique (client->unique);

  VALIDATE_PROC (proc);

  OK;
}
