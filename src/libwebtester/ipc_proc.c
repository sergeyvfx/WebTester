/**
 * WebTester Server - server of on-line testing system
 *
 * IPC procedures stuff module
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "ipc.h"
#include "sock.h"

/**
 * Handler of command `exit`
 *
 * @param __argc - number of arguments
 * @param __argv - array of arguments
 * @return zero on success, non-zero otherwise
 */
int
ipc_proc_exit (int __argc, char **__argv)
{
  if (__argc != 1)
    {
      IPC_PROC_ANSWER ("-ERR Usaage: `exit'\n");
      return 0;
    }

  IPC_PROC_ANSWER ("+OK ");

  ipc_disconnect_client (ipc_get_current_client (), 1);

  return 0;
}
