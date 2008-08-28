/*
 *
 * ================================================================================
 *  ipc_builtin.c - part of the WebTester Server
 * ================================================================================
 *
 *  IPC builtin functions
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "autoinc.h"
#include  "ipc.h"

#include <libwebtester/ipc.h>
#include <libwebtester/md5.h>
#include <libwebtester/core.h>

static char *default_procs[][2]={
    {"exit",   "Close connection to server"},
    {"help",   "Prints this message"},
    {"login",  "Login to system"},
    {"tail",   "Print tail of CORE output buffer"},
    {"uptime", "CORE's uptime"},
    {0, 0}
  };

#define PASSWD_SALT "#RANDOM#"

int
wt_ipc_help                        (int __argc, char **__argv)
{
  int i=0;

  if (__argc>1)
    {
      IPC_PROC_ANSWER ("-ERR Usage: `help`\n");
      return 0;
    }

  IPC_PROC_ANSWER ("+OK WebTester Server IPC console help\n\nList of default registered procedures:\n");

  while (default_procs[i][0])
    {
      IPC_PROC_ANSWER ("  %s\t\t - %s\n", default_procs[i][0], default_procs[i][1]);
      i++;
    }

  IPC_PROC_ANSWER ("When you run difficult procedures without parameters, you'll receive short help about usage of this procedures.\n");

  return 0;
}

int
wt_ipc_login                       (int __argc, char **__argv)
{
  ipc_client_t *client;
  void *node=0;
  char prefix[1024], dummy[1024], pass[1024]={0};

  client=ipc_get_current_client ();

  if (__argc!=3)
    {
      IPC_PROC_ANSWER ("-ERR Usage: `login <OGIN> <ASSWORD>`\n");
      return 0;
    }

  if (client->authontificated)
    {
      IPC_PROC_ANSWER ("-ERR Ypu have been already authontificated\n");
      return 0;
    }

  sprintf (prefix, "Server/IPC/Users/%s", __argv[1]);
  CONFIG_OPEN_KEY (node, prefix);
  if (!node)
    goto __fail_;

  sprintf (dummy, "%s/password-hash", prefix);
  CONFIG_PCHAR_KEY (pass, dummy);
  md5_crypt (__argv[2], PASSWD_SALT, dummy);
  if (strcmp (pass, dummy+8))
    goto __fail_;

  sprintf (dummy, "%s/access", prefix);

  client->authontificated=1;
  strcpy (client->login, __argv[1]);
  CONFIG_INT_KEY (client->access, dummy);

  IPC_PROC_ANSWER ("+OK\n");

  return 0;
__fail_:
  IPC_PROC_ANSWER ("-ERR Bad OGIN or ASSWORD\n");
  return 0;
}

int
wt_ipc_logout                      (int __argc, char **__argv)
{
  ipc_client_t *client=ipc_get_current_client ();
  
  if (!client->authontificated)
    {
      IPC_PROC_ANSWER ("-ERR You didn't pass authontification\n");
      return 0;
    }

  client->authontificated=0;
  strcpy (client->login, "");
  client->access=0;
  
  IPC_PROC_ANSWER ("+OK\n");

  return 0;
}

int
wt_ipc_tail                        (int __argc, char **__argv)
{
  char **lines;
  int count, i;

  IPC_ADMIN_REQUIRED

  lines=core_output_buffer (&count);

  IPC_PROC_ANSWER ("+OK ");

  for (i=0; i<count; ++i)
    IPC_PROC_ANSWER ("%s", lines[i]);

  return 0;
}

int
wt_ipc_uptime                      (int __argc, char **__argv)
{
  time_t uptime;
  unsigned long s, m, h;
  ipc_client_t *client=ipc_get_current_client ();

  if (!client->authontificated)
    {
      IPC_PROC_ANSWER ("-ERR You must be authentificated to run this command\n");
      return 0;
    }

  uptime=core_get_uptime ();

  s=uptime%60; uptime/=60;
  m=uptime%60; uptime/=60;
  h=uptime;

  IPC_PROC_ANSWER ("+OK %ld:%02ld:%02ld\n", h, m, s);

  return 0;
}
