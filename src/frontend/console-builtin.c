/*
 *
 * ================================================================================
 *  console-builtin.c - part of the WebTester Server Frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "console.h"
#include "builtin.h"

#include <libwebtester/sock.h>
#include <libwebtester/strlib.h>

////
//

#define MSG_DISCONNECT(_msg) \
    (!strncmp (_msg, "The system is SHUTTING DOWN.", 28))

////
//

static cmd_context_t *cmd_context;

////////
//

static int
cmd_help                           (int __argc, char **__argv)
{
  if (__argc!=1)
    {
      console_log ("Usage: help\n");
      return 0;
    }

  console_log ("WebTester Server Console help\n"
               "Default registered function:\n"
               "  connect    - connect to server\n"
               "  disconnect - disconnect from server\n"
               "  help       - display this message\n"
               "  ipc        - send an IPC command\n"
               "If you want to send an IPC command, simple type `ipc <ipc command>`. If <ipc command> contains spaces - include this command into qoutes\n");

  return 0;
}

static int
cmd_ipc                            (int __argc, char **__argv)
{
  char cmd[4096], dummy[1024];
  int sock=console_get_socket ();

  if (__argc!=2)
    {
      console_log ("Usage: ipc <ipc command>\n");
      return 0;
    }

  strcpy (cmd, __argv[1]);

  if (!strcmp (cmd, ""))
    return 0;

  if (sock>0)
    {
      char ans[65536];
      int a;
      if ((a=sock_answer (sock, "%s\n", cmd)))
        goto __fail_;

      if (sock_read (sock, 65536, ans)<0 || MSG_DISCONNECT (ans))
        goto __fail_;

      console_log ("%s", ans);
    } else
      console_log ("Not connected to server.\n");

  trim (cmd, dummy);
  if (!strcmp (dummy, "exit"))
    disconnect_from_server ();

  return 0;
__fail_:
  disconnect_from_server ();
  return 0;
}

static int
cmd_connect                        (int __argc, char **__argv)
{
  if (__argc==2)
    {
      connect_to_server (__argv[1], FALSE, "", "");
    } else
  if (__argc==4)
    {
      connect_to_server (__argv[1], TRUE, __argv[2], __argv[3]);
    } else
      console_log ("Usage: connect <server>:<port> [<ogin> <assword>]\n");

  return 0;
}

static int
cmd_disconnect                      (int __argc, char **__argv)
{
  if (__argc!=1)
    console_log ("Usage: disconnect\n"); else
    disconnect_from_server ();

  return 0;
}

////////
// User's backend

BOOL
console_builtin_init               (void)
{
  cmd_context=cmd_create_context ("CONSOLE_CONTEXT");

  console_proc_register ("help",         cmd_help);
  console_proc_register ("ipc",          cmd_ipc);
  console_proc_register ("connect",      cmd_connect);
  console_proc_register ("disconnect",   cmd_disconnect);

  return TRUE;
}

void
console_builtin_done               (void)
{
  if (cmd_context)
    cmd_destroy_context (cmd_context);
}

int
console_proc_exec                  (char *__cmd)
{
  int argc=0, res=-1;
  char **argv=0;
  if (__cmd[0]==0) return 0;

  cmd_parse_buf (__cmd, &argv, &argc);
  if (argc>0)
    res=cmd_context_execute_proc (cmd_context, argv, argc);
  cmd_free_arglist (argv, argc);
  return res;
}

int
console_proc_register              (char *__procname, cmd_entry_point __entrypoint)
{
  cmd_context_proc_register (cmd_context, __procname, __entrypoint);
  return 0;
}
