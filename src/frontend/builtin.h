/*
 *
 * ================================================================================
 *  builtin.h - part of the WebTester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_builtin_h_
#define _wt_builtin_h_

#include <libwebtester/types.h>

////
//

typedef struct {
  struct {
    char data[65536];
    long length;
  } body;

  __u32_t type;
} ipc_ans_t;

#define IAT_UNKNOWN  0x0000
#define IAT_OK       0x0001
#define IAT_ERR      0x0002

#define IPC_ANS_LEN(__self)   ((__self).body.length)
#define IPC_ANS_BODY(__self)  ((__self).body.data)

////////
//

BOOL
parse_args                         (int __argc, char **__argv);

BOOL
load_config_file                   (void);

BOOL
connect_to_server                  (char *__server, BOOL __login_at_connect, char *__login, char *__pass);

void
disconnect_from_server             (void);

int
read_ipc_sock                      (int __sock, ipc_ans_t *__ans);

void
bind_button                       (int __i, char *__caption, char *__command);

void
exec_button_command               (int __i);

#endif
