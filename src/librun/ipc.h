/*
 * ================================================================================
 *  ipc.h - part of the LibRUN
 * ================================================================================
 *
 * IPC interface for LibRUN
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _run_ipc_h_
#define _run_ipc_h_

#include <libwebtester/smartinclude.h>
#include <libwebtester/cmd.h>
#include <libwebtester/types.h>

#include <librun/run.h>

////////////////////////////////////////
//

// Default IPC port
#define RUN_IPC_PORT         13667
// Default IPC host to bint to
#define RUN_IPC_HOST         "127.0.0.1"
// Default IPC client to connect to
#define RUN_IPC_CLIENT_HOST  "127.0.0.1"

// MAX IPC socket  stack size
#define RUN_SOCK_STACK_SIZE   1024
// MAX count of clients
#define RUN_IPC_MAX_CLIENTS   1024

// Max timeout of keeping client registered
#define RUN_IPC_MAX_KEEP_TIMEOUT 120.0

////////////////////////////////////////
// Type defenitions

typedef struct {
  int num;    // Num in array
  int sock;   // Accepted socked

  long unique;
  char security_key[RUN_KEY_LENGTH];

  char cmd[RUN_SOCK_STACK_SIZE];

  timeval_t timestamp;
} run_ipc_client;

////////
//

int             // Initialize IPC stuff
run_ipc_init                       (void);

void            // Uninitialize IPC stff
run_ipc_done                       (void);

unsigned int
run_ipc_port                       (void);

char*           // Host to connect to from client
run_ipc_client_host                (void);

run_ipc_client* // Current IPC user
run_ipc_current_user               (void);

void            // Register IPC proc at context
run_ipc_register_proc              (char *__name, cmd_entry_point __entry_point);

int             // Answer to current client
run_ipc_socket_answer              (char *__text, ...);

BOOL            // Unregister client by unique
run_ipc_unregister_by_unique       (int __unique);

////////
//

int
run_ipc_listen                     (void);

int
run_ipc_interact                   (void);

////////////////////////////////////////
// IPC built-in

int
run_ipc_proc_unique                (int __argc, char **__argv);

int
run_ipc_proc_security              (int __argc, char **__argv);

int
run_ipc_proc_lrvmpid               (int __argc, char **__argv);

int
run_ipc_proc_taskpid               (int __argc, char **__argv);

int
run_ipc_proc_finalize              (int __argc, char **__argv);

int
run_ipc_proc_exit_code             (int __argc, char **__argv);

int
run_ipc_proc_termsig               (int __argc, char **__argv);

int
run_ipc_proc_stopsig               (int __argc, char **__argv);

int
run_ipc_proc_continue              (int __argc, char **__argv);

int
run_ipc_proc_base                  (int __argc, char **__argv);

int
run_ipc_proc_exec_error            (int __argc, char **__argv);

int
run_ipc_proc_close_pipe            (int __argc, char **__argv);

int
run_ipc_proc_validate              (int __argc, char **__argv);

#endif
