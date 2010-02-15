/**
 * WebTester Server - server of on-line testing system
 *
 * LibRUN - library for profilled running processes
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "run.h"
#include "hv.h"
#include "unique.h"
#include "ipc.h"

#include <libwebtester/dynastruc.h>
#include <libwebtester/conf.h>
#include <libwebtester/fs.h>
#include <libwebtester/md5.h>
#include <libwebtester/log.h>
#include <libwebtester/mutex.h>

#include <stdio.h>

#include <glib.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <time.h>

#define SECURITY_MAGICK    "asdadfad"

/* Set process error of executing and set description */
#define SET_PROCESS_EXECERROR(__self,__desc) \
  { \
    (__self).state=PS_EXECERROR; \
    strcpy ((__self).err_desc,  __desc); \
  }

/* But is it OK? */
#define ACC_RSSMEM_USAGE(__t)  (__t.hiwater_rss)
#define ACC_TIME_USAGE(__t) \
  (MAX (MAX (__t.ac_etime, __t.ac_stime), __t.ac_utime))

/* Check process limits and update status */
#define CHECK_LIMITS(__proc, __state) \
  { \
    /*  Check for RSS memory limit */ \
    if ((__proc).limits.rss_mem>0 && \
        (__proc).limits.rss_mem<RUN_PROC_RSSUSAGE (__proc)) \
      __state|=PS_MEMORYLIMIT; \
    /* Check for elapsed time limit */  \
    if ((__proc).limits.time>0 && \
        (__proc).limits.time<RUN_PROC_TIMEUSAGE (__proc)) \
      __state|=PS_TIMELIMIT; \
\
    /* Check for peak limits */ \
    if (peak_rss>0 && peak_rss<RUN_PROC_RSSUSAGE (__proc)) \
      __state|=PS_MEMORYLIMIT; \
    if (peak_time>0 && peak_time<RUN_PROC_TIMEUSAGE (__proc)) \
      __state|=PS_TIMELIMIT; \
  }

/* LibRUN main belts. All profiling processes */
/* are put to this belts. */
static dynastruc_t *belts = NULL;

/*
 * Make convertion secs to usecs here to optimize
 * future evalutions
 */

/* Timeout for answer from LRVM */
static DWORD lrvm_ans_timeout = LRVM_ANS_TIMEOUT*USEC_COUNT;

/* Delay in sock. reading stuff in LRVM */
static double lrvm_sock_read_delay = LRVM_SOCK_READ_DELAY;

/* Timeout for reading socket in LRVM */
static double lrvm_sock_read_timeout = LRVM_SOCK_READ_TIMEOUT;

static char lrvm_config_file[4096] = {0};

/* Timeout for accept process's zombie */
static DWORD zombie_timeout = RUN_ZOMBIE_TIMEOUT*USEC_COUNT;

/* Time and memory corrections */
static long rss_correction = 0;
static long time_correction = 0;

/* Mutex for belts */
static GMutex *belts_mutex = NULL;

static DWORD peak_time = RUN_PEAK_TIME*USEC_COUNT;
static DWORD peak_rss = RUN_PEAK_RSS;

static long uid = 0;
static long gid = 0;

/****
 * Some stupid stuff
 */

/**
 * Zerolize process info struct
 *
 * @param __self - process info to be zerolized
 */
static void
zerolize_process (run_process_info_t *__self)
{
  memset (__self, 0, sizeof (run_process_info_t));
  __self->state = PS_PENDING;
  __self->unique = -1;

  __self->uid = uid;
  __self->gid = gid;
}

/**
 * Set process's limits informztion
 */
static void
set_limit_inrormation (run_process_info_t *__self,
                       DWORD __rss_limit, DWORD __time_limit)
{
  /* Stupid filling of structure */
  __self->limits.rss_mem = __rss_limit;
  __self->limits.time = __time_limit;
}

/**
 * Generate sequrity hash
 *
 * @param __self - buffer to save generated hash
 */
static void
generate_security_hash (char *__self)
{
  char ptime[RUN_HASH_LENGTH];

  snprintf (ptime, BUF_SIZE (ptime), "%ld", time (0) + rand () % 8192);

  /* Just some random 8 characters */
  md5_crypt (ptime, "wl3mw0sq", __self);
}

/**
 * Fill process's sequrity info
 *
 * @param __self - ptocess to operate with
 */
static void
fill_security_info (run_process_info_t *__self)
{
  char pchar[RUN_HASH_LENGTH + 64], dummy[128];
  generate_security_hash (__self->security_hash);

  snprintf (pchar, BUF_SIZE (pchar), "##%s@%ld##",
            __self->security_hash, __self->unique);
  md5_crypt (pchar, SECURITY_MAGICK, dummy);
  strcpy (__self->security_key, dummy + 8);
  memset (pchar, 0, sizeof (pchar));
}

/**
 * Spawn new process descriptor
 *
 * @param __cmd - command to run
 * @param __workdir - working directory
 * @param __rss_limit - limit of RSS
 * @param __time_limit - time limit
 * @return new descriptor
 */
static run_process_info_t*
spawn_new_process (const char *__cmd, const char *__workdir,
                   DWORD __rss_limit, DWORD __time_limit)
{
  run_process_info_t *process;
  process = malloc (sizeof (run_process_info_t));

  zerolize_process (process);

  process->unique = run_unique_alloc ();
  fill_security_info (process);

  set_limit_inrormation (process, __rss_limit, __time_limit);

  strncpy (process->cmd, __cmd, RUN_CMD_LEN);
  strncpy (process->workdir, __workdir, RUN_DIR_LEN);

  return process;
}

/****
 * Pipe
 */

/**
 * Append next buffer to processe's pipe buffer
 *
 * @param __self - process for which buffer should belong to
 * @param __buf - buffer tail
 */
static void
proc_append_pipe_buf (run_process_info_t *__self, const char *__buf)
{
  int alen = strlen (__buf);
  char *spchar = 0;

  spchar = __self->pipe_buf.pchar;
  __self->pipe_buf.pchar = malloc (__self->pipe_buf.len + alen + 1);
  strcpy (__self->pipe_buf.pchar, "");
  if (spchar)
    {
      strcpy (__self->pipe_buf.pchar, spchar);
      free (spchar);
    }
  strcat (__self->pipe_buf.pchar, __buf);
  __self->pipe_buf.len += alen;
}

/**
 * Read all data from process's pipe
 *
 * @param __self - process to read all info from
 */
static void
read_process_pipe (run_process_info_t *__self)
{
  int ch, len = 0;
  char buf[RUN_PIPE_READ_BUF_LEN];

  DEBUG_LOG ("librun", "Reading process's pipe...\n");

  /* There is no pipe or pipe is closed */
  if (!__self->pipe)
    {
      return;
    }

  /* Nothing serious in this. */
  /* May pipe has been closed by other side or system. */
  if (ferror (__self->pipe) || fileno (__self->pipe) <= 0)
    {
      return;
    }

  /*
   * TODO: We don't need lock the process, because in case of deadlock
   *       we would do nothing in belts overview stuff
   */

  /* Zerolization of buffer */
  memset (buf, 0, sizeof (buf));

  ch = fgetc (__self->pipe);
  while (ch != EOF)
    {
      buf[len++] = ch;
      if (len == RUN_PIPE_READ_BUF_LEN - 1)
        {
          buf[len] = 0;
          proc_append_pipe_buf (__self, buf);
          len = 0;
        }
      ch = fgetc (__self->pipe);
    }

  if (len)
    {
      buf[len] = 0;
      proc_append_pipe_buf (__self, buf);
    }
}

/**
 * Close process's pipe
 *
 * @param __self - process which pipe will be closed
 */
static void
proc_close_pipe (run_process_info_t *__self)
{
  /*
   * TODO: Need this stuff to stop errors, caused by
   * nultiply closing of instance.
   *
   * We couldn't use mutexes, because in sthis way
   * we'll have to associate it with process structure,
   * but it is a lot of additional memory.
   *
   * But think about this. (May be troubles)
   */

  /*
   * TODO: Add safe thread block here
   */

  DEBUG_LOG ("librun", "Closing process's pipe...\n");

  /* <!-- BEGIN SAFE BLOCK --!> */
  if (RUN_PROC_TEST_FLAG (*__self, PF_PIPE_CLOSING)) return;
  RUN_PROC_SET_FLAG (*__self, PF_PIPE_CLOSING);
  RUN_PROC_LOCK (*__self);
  /* <!-- END SAFE BLOCK --!> */

  if (__self->pipe)
    {
      pclose (__self->pipe);
      __self->pipe = 0;
    }

  RUN_PROC_UNLOCK (*__self);
  RUN_PROC_FREE_FLAG (*__self, PF_PIPE_CLOSING);
}

/**
 * Fill resources usage from stats stucture
 *
 * @param __self - process to fill
 * @param __ststs - information to fill form
 */
static void
proc_fill_from_stats (run_process_info_t *__self, struct taskstats __stats)
{
  DWORD dummy;

  /*
   * TODO: Some troubles with base memory because
   *       of optimizer and memory manager.
   */

  __self->r_usage.rss_mem = ACC_RSSMEM_USAGE (__stats) -
          __self->r_base.rss_mem * 0;

  /*
   * TODO; But maybe we shuold zerolize RSS memory usage in negative situation?
   */

  if (!(rss_correction < 0 && __self->r_usage.rss_mem<-rss_correction))
    {
      __self->r_usage.rss_mem += rss_correction;
    }

  dummy = ACC_TIME_USAGE (__stats);

  /* But are we really need this? */
  if (dummy >= __self->r_base.time - time_correction)
    {
      __self->r_usage.time = dummy - __self->r_base.time + time_correction;
    }
  else
    {
      __self->r_usage.time = 0;
    }
}

/**
 * ReadLRVM PID from pipe
 *
 * @oaram __self - process to read LRVM PID for
 */
static void
proc_read_lrvm_pid (run_process_info_t *__self)
{
  int ch;
  __u32 pid = 0;

  DEBUG_LOG ("librun", "Reading LRVM's pid from pipe of process %ld...\n",
             __self->unique);

  if (!__self || !__self->pipe)
    {
      DEBUG_LOG ("librun", "Error reading LRVM PID from pipe. "
                            "__self=%ld, __self->pipe=%ld\n",
                 __self, ((__self) ? (__self->pipe) : (0)));
      return;
    }

  /*
   * TODO: We don't have to lock process here, because in case
   *       of some errors in LRVM we'll dead-lock hypervisor
   */

  ch = fgetc (__self->pipe);
  while (ch >= '0' && ch <= '9')
    {
      pid = pid * 10 + ch - '0';
      ch = fgetc (__self->pipe);
    }

  __self->lrvm_pid = pid;

  DEBUG_LOG ("librun", "Read LRVM's pid from pipe of process %ld: %u\n",
             __self->unique, pid);
}

/****
 * Belts
 */

/**
 * Dyna comparator by unique
 *
 * @param __l - process
 * @param __r - needed unique
 * @return non-zero if process is wanted, zero otherwise
 */
static int
belts_unique_comparator (void *__l, void *__r)
{
  run_process_info_t* self = __l;
  return self->unique == *(long*) __r;
}

/**
 * Dya deleter of process
 *
 * @param __self - process to delete
 */
static void
belts_dyna_deleter (void *__self)
{
  /*
   * Use this just killing, because after killing
   * caller of this process will continue working
   * and may call run_free_process().
   */

  run_kill_process (__self, 0);
}

/*
 * TODO: Use mutexes in all operations with belts.
 *       May be this will stop that strange segfolt?
 */

/**
 * Append process to LibRUN's belts
 *
 * @param __self - process to append
 */
static void
belts_append (run_process_info_t* __self)
{
  mutex_lock (belts_mutex);

  /* Save timestamp */
  __self->timestamp = now ();
  dyna_append (belts, __self, 0);
  RUN_PROC_SET_FLAG (*__self, PF_INBELTS);

  mutex_unlock (belts_mutex);
}

/**
 * Remove process from LibRUN's belts
 *
 * @oaram __self - process to remove
 */
static void
belts_remove (run_process_info_t* __self)
{
  dyna_item_t *item;
  mutex_lock (belts_mutex);

  if (RUN_PROC_TEST_FLAG (*__self, PF_INBELTS))
    {
      dyna_search_reset (belts);
      item = dyna_search (belts, &__self->unique, 0, belts_unique_comparator);

      if (item)
        {
          /* If requested item has been found */
          dyna_delete (belts, item, 0);
        }

      RUN_PROC_FREE_FLAG (*__self, PF_INBELTS);
    }

  mutex_unlock (belts_mutex);
}

/****
 *
 */

/**
 * Kill child executing process
 *
 * @param __self - process which child will be killed
 */
static void
kill_child_process (run_process_info_t *__self)
{
  /*
   * Need this function because profiling process is may be started under
   * different acess previlegies, so we can't just call the kill() function
   */

  if (!__self)
    {
      return;
    }

  if (!__self->uid && !__self->gid)
    {
      /* There is no changing of previlegies, so we can kill */
      /* the process by casting kill() */

      core_kill_process (__self->task_pid, SIG_TERM);
    }
  else
    {
      char add[1024];

      strcpy (add, "");
      if (strcmp (lrvm_config_file, ""))
        {
          strcat (add, " -config-file ");
          strcat (add, lrvm_config_file);
        }

      sys_launch ("%s/sbin/lrvm_kill -uid %ld -gid %ld -pid %u -signal %d%s",
                  HOME_DIRECTORY,
                  __self->uid, __self->gid, __self->task_pid, SIG_TERM, add);
    }

}

/****
 * Belts overview stuff
 */

/**
 * Check timeout to answer from LRVM
 *
 * @param __self - process to check
 * @param __now - current timestamp
 */
static void
belts_check_lrvm_ans_timeout (run_process_info_t *__self, timeval_t __now)
{
  if (!__self) return;
  if ((__self->lrvm_pid <= 0 ||
          __self->task_pid <= 0) && /* LRVM not answered yet */
      (tv_usec_cmp (__self->timestamp, 0) > 0) && /* Because of some */
                                                  /* strange bugs with */
                                                  /* SMT technology */
                                                  /* No answer for too long */
      (tv_usec_cmp (timedist (__self->timestamp, __now),
                    lrvm_ans_timeout) >= 0)
      )
    {
      /*
       * PID of LRVM has not been just set and timeout to answer from LRVM
       * exceeded, so there is some executing error.
       */

      RUN_PROC_LOCK (*__self);
      SET_PROCESS_EXECERROR (*__self,
                             "Timelimit to answer from LRVM exceeded.");
      run_kill_process (__self, 0);
      RUN_PROC_UNLOCK (*__self);
    }
}

/**
 * Check process's zombie
 *
 * @param __self - process to check
 * @param __now - current timestamp
 */
static void
belts_check_proc_zombie (run_process_info_t *__self, timeval_t __now)
{
  /* Set flag and wait for a moment.. */
  if (!RUN_PROC_TEST_FLAG (*__self, PF_ZOMBIED))
    {
      __self->zombie_timestamp = __now;
      RUN_PROC_SET_FLAG (*__self, PF_ZOMBIED);
    }

  /* No answer for too long */
  if (tv_usec_cmp (timedist (__self->zombie_timestamp, __now),
                   zombie_timeout) >= 0)
    {
      /* There is no info for too long */
      RUN_PROC_LOCK (*__self);
      SET_PROCESS_EXECERROR (*__self, "Suspesion in zombie");
      run_kill_process (__self, 0);
      RUN_PROC_UNLOCK (*__self);
    }
}

/**
 * Check rusage limits
 *
 * @param __self - ptocess to check
 * @param __stats - statistics of rusage
 */
static void
belts_check_limits (run_process_info_t *__self, struct taskstats __stats)
{
  run_proc_state_t state = 0;
  proc_fill_from_stats (__self, __stats); /* Update stats info  */
  CHECK_LIMITS (*__self, state);
  if (state)
    {
      RUN_PROC_LOCK (*__self);
      run_kill_process (__self, state);
      RUN_PROC_UNLOCK (*__self);
    }
}

/**
 * Get some parameters from config file
 */
static void
read_config (void)
{
  char dummy[4096];
  double t = 0;

  /* Corrections */
  CONFIG_INT_KEY (rss_correction, "LibRUN/Corrections/RSS");
  CONFIG_INT_KEY (time_correction, "LibRUN/Corrections/Time");
  time_correction *= USEC_COUNT;

  /* Hypervisor timeouts */
  t = 0;
  CONFIG_FLOAT_KEY (t, "LibRUN/HyperVisor/ZombieTimeout");
  if (t > 0) zombie_timeout = t * USEC_COUNT; /* Microseconds */

  /* LRVM configurations */
  t = 0;
  CONFIG_FLOAT_KEY (t, "LibRUN/LRVM/AnsTimeout");
  if (t > 0) lrvm_ans_timeout = t * USEC_COUNT; /* Microseconds */

  t = 0;
  CONFIG_FLOAT_KEY (t, "LibRUN/LRVM/SockReadTimeout");
  if (t > 0) lrvm_sock_read_timeout = t; /* AS-IS */

  t = 0;
  CONFIG_FLOAT_KEY (t, "LibRUN/LRVM/SockReadDelay");
  if (t > 0) lrvm_sock_read_delay = t; /* AS-IS */

  CONFIG_PCHAR_KEY (lrvm_config_file, "LibRUN/LRVM/ConfigFile");

  /* Peak resource usages */
  t = 0;
  CONFIG_FLOAT_KEY (t, "LibRUN/Limits/Time");
  peak_time = t*USEC_COUNT; /* Microseconds */

  CONFIG_INT_KEY (peak_rss, "LibRUN/Limits/RSS");

  strcpy (dummy, "");
  CONFIG_PCHAR_KEY (dummy, "LibRUN/DefaultUser");
  uid = uid_by_name (dummy);

  strcpy (dummy, "");
  CONFIG_PCHAR_KEY (dummy, "LibRUN/DefaultGroup");
  gid = gid_by_name (dummy);
}

/**
 * Unpack command and check process' existment
 *
 * @param __cmd - command to unpack
 * @param __workdir - working directory
 * @param __out - buffer for unpacked command
 * @return non-zero if process exists, zero otherwise
 */
static BOOL
unpack_cmd (const char *__cmd, const char *__workdir, char *__out)
{
  int argc = 0;
  char **argv = 0;
  BOOL res = FALSE;

  DEBUG_LOG ("librun", "Unpacking command...\n");

  strcpy (__out, __cmd);

  /* Get first param of command line (file to execute) */
  cmd_parse_buf (__cmd, &argv, &argc);

  /* Check for existment of this file */
  if (!fexists (argv[0]))
    {
      char dummy[RUN_CMD_LEN];

      /* Check for executable in working directory */
      snprintf (dummy, BUF_SIZE (dummy), "%s/%s", __workdir, argv[0]);
      if (!fexists (dummy))
        {
          /* Check for executable in registered paths */
          char *ptr = get_full_file (argv[0]);
          if (!strcmp (argv[0], ptr))
            {
            }
          else
            {
              dirname (ptr, ptr);
              snprintf (dummy, BUF_SIZE (dummy), "%s/%s", ptr, __cmd);
              strcpy (__out, dummy);
              res = TRUE;
            }
          free (ptr);
        }
      else
        {
          res = TRUE;
        }
    }
  else
    {
      res = TRUE;
    }

  cmd_free_arglist (argv, argc);

  DEBUG_LOG ("librun", "Command unpacked\n");

  return res;
}

/**
 * Executing stuff
 *
 * @param __data - process descriptor
 */
static gpointer
execute_thread (gpointer __data)
{
  char unpacked_cmd[RUN_CMD_LEN];
  char cmd_arg[RUN_CMD_LEN],
       workdir_arg[RUN_CMD_LEN],
       cmd[RUN_CMD_LEN], add[1024] = {0};

  run_process_info_t *task = __data;
  RUN_PROC_STATE (*task) = PS_RUNNING;

  DEBUG_LOG ("librun", "Enter executing thread for process %ld\n",
             task->unique);

  /* Save timestamp of execution and append to belts */
  DEBUG_LOG ("librun", "Appending to belts task %ld...\n", task->unique);
  belts_append (task);

  if (!unpack_cmd (task->cmd, task->workdir, unpacked_cmd))
    {
      SET_PROCESS_EXECERROR (*task, "Unable find file to execute");
      g_thread_exit (0);
    }

  /* Preparsing arguments' strings */
  prepare_cmdarg (unpacked_cmd, cmd_arg);
  prepare_cmdarg (task->workdir, workdir_arg);

  if (strcmp (lrvm_config_file, ""))
    {
      strcat (add, " -config-file ");
      strcat (add, lrvm_config_file);
    }

  if (RUN_PROC_TEST_FLAG (*task, PF_CHROOT))
    {
      strcat (add, " -chroot");
    }

  snprintf (cmd, BUF_SIZE (cmd),
            "%s -cmd %s -workdir %s -security %s -host %s -port %u "
            "-unique %ld -sock-read-timeout %lf -sock-read-delay %lf "
            "-uid %ld -gid %ld %s 2>&1",
            LRVM_FULL_CMD, cmd_arg, workdir_arg, task->security_hash,
            run_ipc_client_host (), run_ipc_port (), task->unique,
            lrvm_sock_read_timeout, lrvm_sock_read_delay,
            task->uid, task->gid, add);

  /* Use pipes to read all messages from process */
  DEBUG_LOG ("librun", "Opening pipe to process %ld... (full cmd: %s) \n",
             task->unique, cmd);
  task->pipe = popen (cmd, "r");
  if (!task->pipe)
    {
      DEBUG_LOG ("librun", "Opening pipe...\n");
      SET_PROCESS_EXECERROR (*task, "Unable to open pipe.");
      g_thread_exit (0);
      return 0;
    }

  proc_read_lrvm_pid (task);
  read_process_pipe (task);
  proc_close_pipe (task);
  belts_remove (task);

  if (task->err_desc && strcmp (task->err_desc, ""))
    {
      task->state |= PS_EXECERROR;
    }

  DEBUG_LOG ("librun", "Leave executing thread for process %ld\n", task->unique);

  g_thread_exit (0);
  return 0;
}

/********
 * User's backend
 */

/**
 * Overview belts
 */
void
run_belts_overview (void)
{
  struct taskstats stats;
  run_process_info_t *proc;
  dyna_item_t *cur;
  timeval_t cur_time;

  /* Lock belts' mutex */
  mutex_lock (belts_mutex);

  cur_time = now ();

  cur = dyna_head (belts);
  while (cur)
    {
      proc = dyna_data (cur);
      cur = dyna_next (cur);

      DEBUG_LOG ("librun", "Overviewving status for process %d...\n",
                 proc->unique);

      if (RUN_PROC_LOCKED (*proc))
        {
          continue;
        }

      /* Check timelimit for answer from LRVM */
      belts_check_lrvm_ans_timeout (proc, cur_time);

      if (proc->lrvm_pid > 0 && proc->task_pid > 0)
        {
          /* Overview all running tasks with non-zero PID */
          if (RUN_PROC_STATE_EXECUTING (*proc))
            {
              if (run_hv_proc_stats (proc->task_pid, &stats))
                {
                  /* There is no no acct information from kernel, but task is */
                  /* unlocked and steel in belts. Do a check stuff */
                  belts_check_proc_zombie (proc, cur_time);
                }
              else
                {
                  /* Got ACCT information */
                  if (RUN_TASKSTATS_ISZOMBIE (stats))
                    {
                      /* Stats may be info about zombie. */
                      /* Set flags and wait for a moment.. */
                      belts_check_proc_zombie (proc, cur_time);
                    }
                  else
                    {
                      /* There is some acct. information, so process couldn't */
                      /* be a zombie */
                      RUN_PROC_FREE_FLAG (*proc, PF_ZOMBIED);

                      /* Check limits */
                      belts_check_limits (proc, stats);
                    }
                } /* if (run_hv_proc_stats (proc->task_pid, &stats)) */
            } /* if (RUN_PROC_STATE_EXECUTING (*proc)) */
        } /* if (proc->lrvm_pid && proc->task_pid) */

      DEBUG_LOG ("librun", "Overviewving status for process %d completed\n",
                 proc->unique);
    }

  /* Unlock belts' mutex */
  mutex_unlock (belts_mutex);
}

/**
 * Initialize RUN STUFF
 *
 * @return zero on success, non-zero otherwise
 */
int
run_init (void)
{
  read_config ();

  /* Check for threads' initialization */
  if (!g_thread_supported ())
    {
      g_thread_init (0);
    }

  belts = dyna_create (); /* Create belts */
  belts_mutex = mutex_create (); /* Create belts's mutex */

  if (run_unique_init ())
    {
      return -1;
    }

  if (run_hypervisor_init ())
    {
      return -1;
    }

  return 0;
}

/**
 * Uninitialize RUN STUFF
 */
void
run_done (void)
{
  run_hypervisor_done ();

  dyna_destroy (belts, belts_dyna_deleter);

  if (belts_mutex)
    {
      mutex_free (belts_mutex);
    }

  run_unique_done ();
}

/**
 * Create process info stucture
 *
 * @param __cmd - command to run
 * @param __workdir - working directory
 * @param __rss_limit - limit of RSS
 * @param __time_limit - time limit
 * @return new descriptor
 */
run_process_info_t*
run_create_process (const char *__cmd, const char *__workdir,
                    DWORD __memory_limit, DWORD __time_limit)
{
  DEBUG_LOG ("librun", "Spawning new process...\n");
  return spawn_new_process (__cmd, __workdir, __memory_limit, __time_limit);
}

/**
 * Set user and group from which processes qre starting
 *
 * @param __self - process to set user and group
 * @param __uid - user ID
 * @param __gid - group ID
 */
void
run_set_usergroup (run_process_info_t *__self, long __uid, long __gid)
{
  DEBUG_LOG ("librun", "Settings process's user:group sequrity...\n");

  if (!__self)
    {
      return;
    }

  __self->uid = __uid;
  __self->gid = __gid;
}

/**
 * Set chroot's enabled
 *
 * @param __self - process to set chroting
 * @param __val - is chrooting enabled?
 */
void
run_set_chroot (run_process_info_t *__self, int __val)
{
  if (__val)
    {
      RUN_PROC_SET_FLAG (*__self, PF_CHROOT);
    }
  else
    {
      RUN_PROC_FREE_FLAG (*__self, PF_CHROOT);
    }
}

/**
 * Free process information
 *
 * @param __self - process to be freed
 */
void
run_free_process (run_process_info_t *__self)
{
  DEBUG_LOG ("librun", "Freeing process %ld...\n", __self->unique);

  if (!__self)
    {
      return;
    }

  run_unique_release (__self->unique);
  proc_close_pipe (__self);
  SAFE_FREE (__self->pipe_buf.pchar);
  free (__self);

  DEBUG_LOG ("librun", "Freeing process completed\n");
}

/**
 * Execute process with profiling
 *
 * @param __self - ptocess to execute
 * @reutrn zero on success, non-zero otherwise
 */
int
run_execute_process (run_process_info_t *__self)
{
  RUN_PROC_STATE (*__self) = PS_STARTING;

  if (__self->unique < 0)
    {
      SET_PROCESS_EXECERROR (*__self, "Unique pool is full");
      return -1;
    }

  if (!(__self->thread = g_thread_create (execute_thread, __self, TRUE, 0)))
    {
      SET_PROCESS_EXECERROR (*__self, "Unable to create testing thread");
      return -1;
    }
  return 0;
}

/**
 * Kill executing process
 *
 * @param __self - process to kill
 * @param __state - state to save
 */
int
run_kill_process (run_process_info_t *__self, int __state)
{
  if (!__self)
    {
      return -1;
    }

  DEBUG_LOG ("librun", "Killing process %ld...\n", __self->unique);

  /* <!-- BEGIN SAFE BLOCK --!> */
  if (RUN_PROC_TEST_FLAG (*__self, PF_KILLING))
    {
      DEBUG_LOG ("librun", "Already killing process %ld\n", __self->unique);
      return -1;
    }
  RUN_PROC_LOCK (*__self);
  RUN_PROC_SET_FLAG (*__self, PF_KILLING);
  /* <!-- END SAFE BLOCK --!> */

  /* For correct error handling in future stuff */
  if (RUN_PROC_EXEC_ERROR (*__self)) __state |= PS_EXECERROR;

  /*
   * TODO: May be some troubles with this
   */

  /* Close connection throug IPC to  */
  /* run_ipc_unregister_by_unique (__self->unique); */

  /* Kill LRVM and it's child process */
  if (__self->task_pid > 0)
    {
      kill_child_process (__self);
      __self->task_pid = 0;
    }

  if (__self->lrvm_pid > 0)
    {
      core_kill_process (__self->lrvm_pid, SIG_TERM);
      __self->lrvm_pid = 0;
    }

  __self->state = PS_KILLED | __state;

  DEBUG_LOG ("librun", "Killed process\n");

  RUN_PROC_FREE_FLAG (*__self, PF_KILLING);
  RUN_PROC_UNLOCK (*__self);
  return 0;
}

/**
 * Finalize process executing
 * (called when profiled process if sucessfully finished)
 *
 * @param __self - ptocess to finalize
 * @return zero on success, non-zero otherwise
 */
int
run_finalize_executing (run_process_info_t *__self)
{
  struct taskstats stats;
  int state = RUN_PROC_GETSIGNAL (*__self);

  if (RUN_PROC_TEST_FLAG (*__self, PF_KILLING))
    {
      DEBUG_LOG ("librun", "Unable to finalize process %ld: "
                           "it is already killing\n",
                 __self->unique);
      return -1;
    }

  DEBUG_LOG ("librun", "Finalizing executing process %ld...\n", __self->unique);

  if (!__self)
    {
      return -1;
    }

  RUN_PROC_LOCK (*__self);

  if (__self->task_pid < 0)
    {
      SET_PROCESS_EXECERROR (*__self, "Task PID is not set");
      return -1;
    }

  /* Getting accounting info */
  /*   Because of `finalize` ipc command is sending by LRVM */
  /*   before it's closing, so task is already finished and */
  /*   we don't need to wait for smth. */
  if (!run_hvpool_stats_by_pid (__self->task_pid, &stats))
    {
      char desc[1024];
      snprintf (desc, BUF_SIZE (desc), "No accounting info found "
                                       "while finalizing %ld. PID: %u\n",
                __self->unique, __self->task_pid);
      SET_PROCESS_EXECERROR (*__self, desc);
      return -1;
    }

  proc_fill_from_stats (__self, stats); /* Update stats info  */
  DEBUG_LOG ("librun", "Process %ld resource usage: RSS: %lld, time: %lld\n",
             __self->unique, RUN_PROC_RSSUSAGE (*__self),
             RUN_PROC_TIMEUSAGE (*__self));
  CHECK_LIMITS (*__self, state); /* Check limits */

  /* run_ipc_unregister_by_unique (__self->unique); */

  __self->state = state | PS_FINISHED;

  DEBUG_LOG ("librun", "Finalized executing process %ld\n", __self->unique);

  RUN_PROC_UNLOCK (*__self);

  return 0;
}

/**
 * Wait for finishing of execution
 *
 * @param __self - ptocess to wait finishing of
 */
void
run_pwait (run_process_info_t *__self)
{
  static struct timespec timestruc;

  timestruc.tv_sec = 0;
  timestruc.tv_nsec = 0.001 * 1000 * 1000 * 1000;

  if (!__self || !__self->thread) return;
  g_thread_join (__self->thread);

  if (RUN_PROC_TEST_FLAG (*__self, PF_INBELTS))
    {
      belts_remove (__self);
    }
}

/****
 *
 */

/**
 * Get process descriptor by unique id
 *
 * @oaram __unique - needed unique
 * @return process descriptor
 */
run_process_info_t*
run_process_info_by_unique (long __unique)
{
  dyna_item_t *item;

  mutex_lock (belts_mutex);
  dyna_search_reset (belts);
  item = dyna_search (belts, &__unique, 0, belts_unique_comparator);
  mutex_unlock (belts_mutex);

  if (!item)
    {
      return NULL;
    }

  return dyna_data (item);
}

/**
 * Some helpers for IPC
 */

/**
 * Process has been terminater
 *
 * @param __self - terminated process
 * @param __sig - signal
 */
void
run_process_terminated (run_process_info_t *__self, int __sig)
{
  if (RUN_PROC_LOCKED (*__self))
    {
      return;
    }

  __self->state = PS_TERMINATED;
  __self->term_sig = __sig;
}

/**
 * Process has been stopped
 *
 * @param __self - stopped process
 * @param __sig - signal
 */
void
run_process_stopped (run_process_info_t *__self, int __sig)
{
  if (RUN_PROC_LOCKED (*__self))
    {
      return;
    }

  __self->state = PS_STOPPED;
  __self->stop_sig = __sig;
}

/**
 * Process has been continued
 *
 * @param __self - continued process
 * @param __sig - signal
 */
void
run_process_continued (run_process_info_t *__self)
{
  if (RUN_PROC_LOCKED (*__self)) return;
  __self->state = PS_CONTINUED;
}

/**
 * Store process base rusage
 *
 * @param __self - process to store rusage
 */
void
run_process_set_base (run_process_info_t *__self)
{
  struct taskstats stats;
  if (!run_hv_proc_stats (__self->task_pid, &stats))
    {
      __self->r_base.time = ACC_TIME_USAGE (stats);
      __self->r_base.rss_mem = ACC_RSSMEM_USAGE (stats);
    }
}
