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

int
ipc_proc_exit                      (int __argc, char **__argv)
{
  if (__argc!=1)
    {
      IPC_PROC_ANSWER ("-ERR Usaage: `exit'\n");
      return 0;
    }
  IPC_PROC_ANSWER ("+OK ");
  ipc_disconnect_client (ipc_get_current_client (), 1);
  return 0;
}
