/*
 *
 * ================================================================================
 *  main.c - part of WebTester Server
 * ================================================================================
 *
 *
 *   ==================
 *  //      /-\       ||  __            __  _________
 * ||       \-/       ||  \ \    /\    / / |  _   _  |
 * ||  /---\___/----\ ||   \ \  /  \  / /  |_| | | |_|
 * ||  \ __      ___/ ||    \ \/ /\ \/ /       | |       
 * ||      \    /     ||     \__/  \__/        |_|
 * ||       \  |      ||
 * ||        | |      ||      WebTester Server
 * ||        \_/    //
 * =================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "autoinc.h"
#include "ipc.h"
#include "library.h"
#include "mainloop.h"
#include "cmdline.h"
#include "core.h"
#include "transport.h"
#include "stat.h"

#include <libwebtester/smartinclude.h>
#include <libwebtester/core.h>
#include <libwebtester/hook.h>
#include <libwebtester/pid.h>
#include <libwebtester/regexp.h>
#include <libwebtester/network-soup.h>
#include <libwebtester/plugin.h>
#include <libwebtester/scheduler.h>
#include <libwebtester/network-smb.h>
#include <libwebtester/log.h>
#include <libwebtester/fs.h>
#include <libwebtester/util.h>
#include <libwebtester/thread.h>

#include <dlfcn.h>
#include <stdio.h>
#include <glib.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>

#include <glib.h>

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

////////
// Some type defenitions

typedef int  (*wt_init_func)       (void);
typedef void (*wt_inst_close_func) (void);

////
//

static BOOL terminating=FALSE;

static char config_file[4096];
static char log_file[4096];

static BOOL good_exit=FALSE;

////
//

static void     // Closes WebTester Server instance
close_instance                     (void);

static void     // Hook for TERM,etc.. signals
signal_term                     (int __signum);

////////////////////////////////////////
//

static long
wt_get_uid                         (void)
{
  struct passwd *p=getpwnam (WEBTESTER_USER);
  if (!p) return -1;
  return (long)p->pw_uid;
}

static long
wt_get_gid                         (void)
{
  struct group *g=getgrnam (WEBTESTER_GROUP);
  if (!g) return -1;
  return (long)g->gr_gid;
}

static void
check_permissions                  (void)
{
  long uid=wt_get_uid (), gid=wt_get_gid ();
  long ruid=getuid (), rgid=getgid ();
  long euid=geteuid (), egid=getegid ();

  // Check for matching of real and effective UID and PID
  if (uid<0 || gid<0 || ruid!=uid || rgid!=gid || euid!=uid || egid!=gid)
    {
      printf ("This module must be run under user %s:%s.\n", WEBTESTER_USER, WEBTESTER_GROUP);
      exit (0);
    }
}

static void
init_iterator                      (char *__inf_str, wt_init_func __funct, char *__err_msg, int __fatal)
{
  core_print (MSG_INFO, "    %s...", __inf_str);
  if (!__funct ())
    {
      CMSG_OK ();
      return;
    }
  CMSG_FAILED_S (__err_msg);
  if (__fatal)
    {
      wt_core_panic ();
    }
}

static void
close_instance_iterator            (char *__inf_str, wt_inst_close_func __func)
{
  core_print (MSG_INFO, "    %s...", __inf_str);
  __func ();
  CMSG_OK ();
}

void
wt_set_config_file                 (char *__self)
{
  strcpy (config_file, __self);
}

void
wt_set_log_file                    (char *__self)
{
  strcpy (log_file, __self);
}

////////////////////////////////////////
//

int             // Initialization of WebTester Server
init_instance                      (void)
{
  // Init threading
  if (thread_init ())
    {
      core_print (MSG_ERROR, "GThreads' stuff is  not supported on your platform.\n    CORE could not be initialized.\n");
      close_instance ();
      return -1;
    }

  core_init ();

  // Print banner
  core_print (MSG_INFO, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
  core_print (MSG_INFO, " %s\n", core_get_version_string ());
  core_print (MSG_INFO, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");

  log_init (log_file);

#ifdef __DEBUG
  core_print (MSG_INFO, "Initializing CORE in DEBUG node...\n");
#else
  if (!core_is_debug_mode ())
    core_print (MSG_INFO, "Initializing CORE...\n"); else
    core_print (MSG_INFO, "Initializing CORE in DEBUG node...\n");
#endif

  // Hook signals
  signal (SIGINT,  signal_term);
  signal (SIGHUP,  signal_term);
//  signal (SIGSTOP, signal_term);
  signal (SIGTERM, signal_term);

  // Chech for multiinstances
  if (create_pid_file (PID_FILE))
    {
      core_print (MSG_INFO, "The WebTester Server is already running.\n");

      // Do not cause a general panic
      good_exit=TRUE;
      return -1;
    }

  init_iterator ("Initializing hooks' stuff", hook_init, "", TRUE);

  core_print (MSG_INFO, "    Loading config file... ");
  if (!config_init (config_file))
    core_print (MSG_INFO, "ok.\n"); else
    core_print (MSG_ERROR, "failed, Using at most default configuration.\n");

#ifdef USER_DEBUG
  {
    char tmp[100]={0};
    CONFIG_PCHAR_KEY (tmp, "CORE/DebugMode");
    if (is_truth (tmp))
      {
        core_print (MSG_INFO, "    Switching CORE into DEBUG mode... ");
        core_enter_debug_mode ();
        core_print (MSG_INFO, "ok.\n");
      }
  }
#endif

  core_register_paths_from_config ();

  init_iterator ("Initializing scheduler",        scheduler_init,        "", FALSE);

  core_print (MSG_INFO, "    Loading plugins...\n");
  wt_load_plugins ();

  core_print (MSG_INFO, "    Loading modules...\n");
  wt_load_modules ();

  init_iterator ("Initializing SAMBA stuff",      samba_init,         "", FALSE);
  init_iterator ("Initializing HTTP stuff",       http_init,          "", FALSE);
  init_iterator ("Initializing transport stuff",  wt_transport_init,  "", FALSE);
  init_iterator ("Initializing IPC stuff",        wt_ipc_init,        "", FALSE);
  init_iterator ("Initializing stat stuff",       wt_stat_init,       "", FALSE);

  init_iterator ("Initializing testing mainloop", wt_mainloop_init,   "", TRUE);

  core_print (MSG_INFO, "CORE initialized. Activating...\n");
  hook_call (CORE_ACTIVATE, 0);
  core_print (MSG_INFO, "CORE activated. %s ready for work.\n", PACKAGE_NAME);
  return 0;
}

static void     // Uninitialization of WebTester Server
close_instance                     (void)
{
  core_print (MSG_INFO, "Deactivating CORE...\n");
  hook_call_backward  (CORE_DEACTIVATE, 0);
  core_print (MSG_INFO, "CORE deactivated.\n");
  core_print (MSG_INFO, "Uninitializing CORE...\n");

  close_instance_iterator ("Uninitializing testing mainloop", wt_mainloop_done);

  close_instance_iterator ("Uninitializing stat stuff",       wt_stat_done);
  close_instance_iterator ("Uninitializing IPC stuff",        wt_ipc_done);
  close_instance_iterator ("Uninitializing transport stuff",  wt_transport_done);
  close_instance_iterator ("Uninitializing HTTP stuff",       http_done);
  close_instance_iterator ("Uninitializing SAMBA stuff",      samba_done);
  close_instance_iterator ("Unloading loaded modules",        wt_unload_modules);

  plugin_unload_all ();

  close_instance_iterator ("Unloading scheduler",             scheduler_done);
  close_instance_iterator ("Unloading hooks" ,                hook_done);

  delete_pid_file (PID_FILE);
  config_done ();

  core_print (MSG_INFO, "Core uninitialized.\n");
  log_done ();
  core_done ();

  thread_done ();
}

void
wt_core_panic                      (void)
{
  core_print (MSG_ERROR, "\nCORE PANIC!\n  Fatal error: %s\n", core_get_last_error ());
  exit (-1);
}

void
wt_core_term                       (void)
{
  if (terminating) return;
  terminating=TRUE;
  close_instance ();
  exit (0);
}

void
signal_term                        (int __signum)
{
  wt_core_term ();
}

////////////////////////////////////////
// MAIN

int
main                               (int __argc, char **__argv)
{
  struct timespec timestruc;

  core_init_version_string ();

  wt_set_config_file (CONFIG_FILE);
  wt_set_log_file    (LOG_FILE);

  wt_cmdline_parse_args (__argc, __argv);

  check_permissions ();

  if (init_instance ())
    return (good_exit)?(0):(-1);

  timestruc.tv_sec=0;
  timestruc.tv_nsec=0.2*NSEC_COUNT; // Nanoseconds :)

  for (;;)
    {
      nanosleep (&timestruc, 0);
    }

  close_instance ();

  return -1;
}
