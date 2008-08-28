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
#include "ipc.h"

#include <libwebtester/ipc.h>
#include <libwebtester/md5.h>
#include <libwebtester/core.h>
#include <libwebtester/assarr.h>
#include <libwebtester/mutex.h>
#include <libwebtester/scheduler.h>

#include <malloc.h>

static char *default_procs[][2]={
    {"exit",   "Close connection to server"},
    {"help",   "Prints this message"},
    {"login",  "Login to system"},
    {"tail",   "Print tail of CORE output buffer"},
    {"uptime", "CORE's uptime"},
    {0, 0}
  };

static long incorrect_login_delay = WT_IPC_INCORRECT_LOGIN_DELAY;

static assarr_t *login_info = NULL;
static long tries_before_lock     = WT_IPC_BLACKLIST_TRIES_BEFORE_LOCK;
static long time_for_count_tries  = WT_IPC_BLACKLIST_TIME_FOR_COUNT_TRIES;
static BOOL blacklist_on_tries_expired = WT_IPC_BLACKLIST_BL_ON_TRIES_EXPIRED;

static mutex_t login_info_mutex=NULL;

static __u64_t login_info_review_interval=WT_IPC_BLACKLIST_REVIEW_LOGIN_INFO_INTERVAL;

typedef struct {
  time_t *access_times;
  int     access_ptr;
} login_info_t;

#define PASSWD_SALT "#RANDOM#"

//////
//

static void
login_info_deleter                 (void *__self)
{
  if (!__self)
    return;

  login_info_t *info=__self;
  SAFE_FREE (info->access_times);
  free (info);
}

static void
read_config                        (void)
{
  char dummy[1024];
  double fdummy;

  fdummy=-1;
  CONFIG_FLOAT_KEY   (fdummy,  "Server/IPC/IncorrectLoginDelay");
  if (fdummy>=0)
    incorrect_login_delay=fdummy*1000;

  fdummy=-1;
  CONFIG_FLOAT_KEY   (fdummy,  "Server/IPC/Blacklisting/ReviewLoginInfoInterval");
  if (fdummy>=0)
    login_info_review_interval=fdummy*USEC_COUNT;

  CONFIG_INT_KEY   (tries_before_lock,     "Server/IPC/Blacklisting/TriesBeforeLock");
  CONFIG_INT_KEY   (time_for_count_tries,  "Server/IPC/Blacklisting/TimeForCountTries");

  CONFIG_PCHAR_KEY (dummy,  "Server/IPC/Blacklisting/BlacklistOnTriesExpired");
  blacklist_on_tries_expired=is_truth (dummy);
}

static void
strip_unused_login_info            (void)
{
  char *key;
  login_info_t *info;
  time_t t=time (0);

  mutex_lock (login_info_mutex);

  ASSARR_FOREACH_DO (login_info, key, info)

    if (info->access_ptr>0 && t-info->access_times[info->access_ptr]>time_for_count_tries)
      assarr_unset_value (login_info, key, login_info_deleter);

  ASSARR_FOREACH_DONE

	mutex_unlock (login_info_mutex);
}

static BOOL
login_allowed                      (char *__ip)
{
  if (!__ip)
    return -1;

  if (tries_before_lock<=0 || time_for_count_tries<0)
    return TRUE;
		
  // We don't wonna to keep trash
  strip_unused_login_info ();

	mutex_lock (login_info_mutex);

  BOOL res=TRUE;
  login_info_t *info=assarr_get_value (login_info, __ip);

  // If tehere is no info, user hadn't tried to login
  if (info && info->access_ptr==tries_before_lock-1 && time (0)-info->access_times[0]<=time_for_count_tries)
    res=FALSE;

	mutex_unlock (login_info_mutex);

  return res;
}

static void
add_login_stamp                    (char *__ip)
{
  if (!__ip)
    return;

  if (tries_before_lock<=0 || time_for_count_tries<0)
    return;

 	mutex_lock (login_info_mutex);

  login_info_t *info=assarr_get_value (login_info, __ip);

  // Create login info if hasn't found
  if (!info)
    {
      info=malloc (sizeof (login_info_t));
      memset (info, 0, sizeof (login_info_t));
      info->access_times=malloc (tries_before_lock*sizeof (time));
      info->access_ptr=-1;
      assarr_set_value (login_info, __ip, info);
    }

  if (info->access_ptr<tries_before_lock-1)
    {
      info->access_ptr++;
    } else {
      int i;
      for (i=0; i<info->access_ptr-1; i++)
        info->access_times[i]=info->access_times[i+1];
    }

  info->access_times[info->access_ptr]=time (0);

	mutex_unlock (login_info_mutex);
}

static void
unset_login_stamps                 (char *__ip)
{
  mutex_lock (login_info_mutex);
  assarr_unset_value (login_info, __ip, login_info_deleter);
  mutex_unlock (login_info_mutex);
}

//////
//

static int
ipc_help                           (int __argc, char **__argv)
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

static int
ipc_login                          (int __argc, char **__argv)
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

  if (!login_allowed (client->ip))
    {
      IPC_PROC_ANSWER ("-ERR You isn't allowed to login to this service. Maybe, you have tried to authontificate to frequently.\n");
      
      if (blacklist_on_tries_expired)
        {
          IPC_PROC_ANSWER ("Your IP has been blacklisted,\n");
          ipc_blacklist_ip (client->ip, -1);
        }
      
      ipc_disconnect_client (client, 1);
      return 0;
    }

  if (client->authontificated)
    {
      IPC_PROC_ANSWER ("-ERR You have been already authontificated\n");
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

  unset_login_stamps (client->ip);

  IPC_PROC_ANSWER ("+OK\n");

  return 0;
__fail_:
  add_login_stamp (client->ip);
  ipc_client_freeze (client, incorrect_login_delay);
  IPC_PROC_ANSWER ("-ERR Bad OGIN or ASSWORD\n");
  return 0;
}

static int
ipc_logout                         (int __argc, char **__argv)
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

static int
ipc_tail                           (int __argc, char **__argv)
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

static int
ipc_uptime                         (int __argc, char **__argv)
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

static int
ipc_ip                             (int __argc, char **__argv)
{
  int i;

  IPC_ADMIN_REQUIRED

  if (__argc<3)
    goto __help_;

  if (!strcmp (__argv[1], "blacklist"))
    {
      for (i=2; i<__argc; i++)
        ipc_blacklist_ip (__argv[i], -1);
      IPC_PROC_ANSWER ("+OK\n");
    } else
  if (!strcmp (__argv[1], "unblacklist"))
    {
      for (i=2; i<__argc; i++)
        ipc_unblacklist_ip (__argv[i]);
      IPC_PROC_ANSWER ("+OK\n");
    } else
      goto __help_;

  return 0;

__help_:
  IPC_PROC_ANSWER ("-ERR Usage: `ip [blacklist|unblacklist] <list of IPs>`\n");
  return 0;
}

static int
ipc_version                        (int __argc, char **__argv)
{
  if (__argc!=1)
    goto __help_;

  IPC_PROC_ANSWER ("+OK %s\n", core_get_version_string ());

  return 0;

__help_:
  IPC_PROC_ANSWER ("-ERR Usage: `vesrion`\n");
  return 0;
}

////
//

static int
login_info_review                  (void *__unused)
{
  strip_unused_login_info ();
  return 0;
}

//////
// User's backend

int
wt_ipc_builtin_init                (void)
{
  read_config ();
	login_info_mutex=mutex_create ();
  login_info=assarr_create ();

  ipc_proc_register ("help",     ipc_help);
  ipc_proc_register ("login",    ipc_login);
  ipc_proc_register ("logout",   ipc_logout);
  ipc_proc_register ("tail",     ipc_tail);
  ipc_proc_register ("uptime",   ipc_uptime);
  ipc_proc_register ("ip",       ipc_ip);
  ipc_proc_register ("version",  ipc_version);

  scheduler_add (login_info_review, 0, login_info_review_interval);

  return 0;
}

void
wt_ipc_builtin_done                (void)
{
  scheduler_remove (login_info_review);
  assarr_destroy (login_info, login_info_deleter);
  mutex_free (login_info_mutex);
}
