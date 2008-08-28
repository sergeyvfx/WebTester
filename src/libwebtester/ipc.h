/*
 *
 * ================================================================================
 *  ipc.c - part of WebTester Server
 * ================================================================================
 *
 *  IPC core stuff module
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _ipc_h_
#define _ipc_h_

#define IPC_MAX_CLIENT_COUNT 1024

#include <libwebtester/cmd.h>
#include <libwebtester/sock.h>

#include <time.h>

typedef struct
{
  int  id;
  int  sock;

//  char cmd[65535];
  char *cmd;

  int  authontificated;
  char login[1024];
  int  access;

  unsigned long flags;

  time_t timestamp;
  char uid[64];
} ipc_client_t;

////////
//

#define IPC_PROC_ANSWER(__text,__args...) \
  sock_answer (ipc_get_current_client ()->sock, __text, ##__args)

#define IPC_PROC_ACCESS \
  (ipc_get_current_client ()->access)

#define IPC_ADMIN_REQUIRED \
  if (IPC_PROC_ACCESS<7) { \
    IPC_PROC_ANSWER ("-ERR You must have administrator previleges to run this command\n"); \
    return 0; \
  }

////////
//

typedef int (*ipc_printf_proc_t)   (const char *__text, ...);

int
ipc_init                           (char *__host, unsigned int __port);

int
ipc_done                           (void);

int
ipc_listen                         (void);

int
ipc_interact                       (void);

int
ipc_disconnect_client              (ipc_client_t *__self, int __send_info);

ipc_client_t*
ipc_get_client_by_id               (int __id);

ipc_client_t*
ipc_get_current_client             (void);

////////////////////////////////////////
// IPC procs' stuff

int
ipc_proc_register                  (char *__procname, cmd_entry_point __entrypoint);

int
ipc_proc_exit                      (int __argc, char **__argv);

#endif
