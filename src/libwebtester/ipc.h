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

typedef struct
{
  int id;
  int sock;
  char cmd[65535];
  
  int authontificated;
  char user_login[1024];
  int user_access;

} ipc_client_t;

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
ipc_get_current_client             (void);

////////////////////////////////////////
// IPC procs' stuff

int
ipc_proc_register                  (char *__procname, cmd_entry_point __entrypoint);

int
ipc_proc_exit                      (int __argc, char **__argv);

#endif
