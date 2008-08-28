/*
 *
 * ================================================================================
 *  ipc-proc.h - part of WebTester Server
 * ================================================================================
 *
 *  Standart IPC procedures' implementation
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "ipc.h"
#include "sock.h"

#define PROC_ANSWER(__text) \
  sock_answer (ipc_get_current_client ()->sock, __text);

int
ipc_proc_exit                      (int __argc, char **__argv)
{
  if (__argc!=1)
    {
      PROC_ANSWER ("Usaage: `exit'\n");
      return 0;
    }
  ipc_disconnect_client (ipc_get_current_client (), 1);
  return 0;
}
