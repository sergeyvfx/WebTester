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

#ifndef _LIBRUN_H_
#define _LIBRUN_H_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/util.h>

#include <sys/types.h>
#include <linux/types.h>
#include <linux/taskstats.h>
#include <stdio.h>
#include <glib.h>

/* State of the process */
#define RUN_PROC_STATE(__self)        ((__self).state)

/* Exit code of the process */
#define RUN_PROC_EXITCODE(__self) \
  ( ((__self).stop_sig) ? ((__self).stop_sig) : \
    ((__self).term_sig) ? ((__self).term_sig) : (__self).exit_code)


#define RUN_PROC_FINISHED(__self)     (RUN_PROC_STATE(__self)&PS_FINISHED)
#define RUN_PROC_TERMINATED(__self)   (RUN_PROC_STATE(__self)&PS_TERMINATED)
#define RUN_PROC_TERMSIG(__self)      ((__self).term_sig)

/* Process's usage of time. usecs */
#define RUN_PROC_TIMEUSAGE(__self)    ((__self).r_usage.time)

/* Process's usage of memory. Kbytes */
#define RUN_PROC_RSSUSAGE(__self)     ((__self).r_usage.rss_mem)

/* Lock the process */
#define RUN_PROC_LOCK(__self)         ((__self).locked=TRUE)

/* Unlock the process */
#define RUN_PROC_UNLOCK(__self)       ((__self).locked=FALSE)

/* Process locked? */
#define RUN_PROC_LOCKED(__self)       ((__self).locked)

/* Buffer from pipe */
#define RUN_PROC_PIPEBUF(__self)      ((__self).pipe_buf.pchar)

/* Length of pipe's buffer */
#define RUN_PROC_PIPEBUF_LEN(__self)  ((__self).pipe_buf.len)

/****
 * Some errors' checking
 */

/* Was memory limit occur? */
#define RUN_PROC_MEMORYLIMIT(__self)  (RUN_PROC_STATE (__self)&PS_MEMORYLIMIT)

/* Was time limit occur? */
#define RUN_PROC_TIMELIMIT(__self)    (RUN_PROC_STATE (__self)&PS_TIMELIMIT)

/* Was there any executing troubles? */
#define RUN_PROC_EXEC_ERROR(__self) (RUN_PROC_STATE (__self)&PS_EXECERROR)

/* Is stats is info about zombied process? */
#define RUN_TASKSTATS_ISZOMBIE(__stats) \
  (!__stats.hiwater_rss && !__stats.hiwater_vm && \
        !__stats.ac_etime && !__stats.ac_utime && !__stats.ac_stime)

/* Get error description */
#define RUN_PROC_ERROR_DESC(__self) ((__self).err_desc)

/* Is process executing at the moment? */
#define RUN_PROC_STATE_EXECUTING(__self) \
  ( RUN_PROC_STATE (__self)&PS_RUNNING   || \
    RUN_PROC_STATE (__self)&PS_STOPPED   || \
    RUN_PROC_STATE (__self)&PS_CONTINUED \
  )

/* Macroses to work with flags */
#define RUN_PROC_SET_FLAG(__self, __flag)   ((__self).flags|=__flag)
#define RUN_PROC_TEST_FLAG(__self, __flag)  ((__self).flags&__flag)
#define RUN_PROC_FREE_FLAG(__self, __flag)  ((__self).flags&=~__flag)

/****
 * Constants
 */

/* MAX command line length */
#define RUN_CMD_LEN         4096

/* MAX filename length */
#define RUN_FN_LEN          1024

/* MAX directory length */
#define RUN_DIR_LEN         4096

/* Timeout to accept process's zombie */
#define RUN_ZOMBIE_TIMEOUT  3.0 /*  secs */

/****
 * LRVM constants
 */

/* Full executable filename of LRVM */
#define LRVM_FULL_CMD           HOME_DIRECTORY "/bin/lrvm"

/* Timeout to tell PID and child's PID */
#define LRVM_ANS_TIMEOUT        2.0     /* secs */

/* Socket read timeout */
#define LRVM_SOCK_READ_TIMEOUT  2.0     /* secs */

/* Socket read delay */
#define LRVM_SOCK_READ_DELAY    0.001   /* secs */

/* Sequrity key length */
#define RUN_KEY_LENGTH        64

/* Security hash len */
#define RUN_HASH_LENGTH       64

/* Length of error description */
#define RUN_ERR_DESC_LEN      1024

/* Length of buffer for read pipe */
#define RUN_PIPE_READ_BUF_LEN 1024

/* Peak resource usages */
#define RUN_PEAK_TIME  (60)         /* secs */
#define RUN_PEAK_RSS   (64*1024)    /* Kbytes */

/****
 * Constant defenitions
 */

#define PS_PENDING     0x0000  /* Process is in pending state */
#define PS_STARTING    0x0001  /* Process is prepearing to start */
#define PS_RUNNING     0x0002  /* Process is running */
#define PS_FINISHED    0x0004  /* Process has been finished */
#define PS_KILLED      0x0008  /* Process has been killed by hypervisor */
#define PS_TERMINATED  0x0010  /* Process has been termintaed by signal */
#define PS_STOPPED     0x0020  /* Process has been stopped by signal */
#define PS_CONTINUED   0x0040  /* Process has been continued by signal */

#define PS_CRASHED     0x0080  /* Process has been crashed */
#define PS_DEADLOCK    0x0100  /* Process caugh dead-lock */
#define PS_TIMELIMIT   0x0200  /* Time limit exceded */
#define PS_MEMORYLIMIT 0x0400  /* Memory limit exceded */

#define PS_EXECERROR   0x0800   /* Some errors while executing */

/****
 * Process's flags
 */
#define PF_ZOMBIED        0x0001
#define PF_PIPE_CLOSING   0x0002
#define PF_KILLING        0x0004
#define PF_CHROOT         0x0008
#define PF_INBELTS        0x0010

/****
 * Type defenitions
 */

typedef __u32_t run_proc_state_t;

/* Limits information */
typedef struct
{
  DWORD rss_mem; /* usecs.  <=0 for unused */
  DWORD time;    /* kbytes. <=0 for unused */
} run_process_limits_t;

typedef struct
{
  /* Input info for profiling */

  char cmd[RUN_CMD_LEN];     /* Command to execute */
  char workdir[RUN_DIR_LEN]; /* Working directory */

  long uid;
  long gid;

  run_process_limits_t limits; /* Process limits */

  /* Information for profiling process */
  run_proc_state_t state; /* State of the process */
  BOOL locked;

  long unique; /* Unique number in pool */
  __u32 lrvm_pid; /* PID of the LRVM */
  __u32 task_pid; /* PID of task */

  __u32_t flags; /* Flags */

  timeval_t timestamp; /* Timestamp of starting to execution */
  timeval_t zombie_timestamp; /* Timestamp when there is no acct. information */

  /* Anti-hacking stuff */
  char security_key[RUN_KEY_LENGTH];
  char security_hash[RUN_HASH_LENGTH];

  /* Process pipe */
  FILE *pipe; /* Pipe to LRVM process */

  struct
  {
    int len;
    char *pchar;
  } pipe_buf; /* Buffer, returned from pipe */

  /* Some parameters to make profiling more accuratly */

  struct
  {
    DWORD time;
    DWORD rss_mem;
  } r_base;

  /* Resultation params */

  int exit_code; /* Process's exit code */
  int term_sig;  /* Signal, which has been caught for termination */
  int stop_sig;  /* Signal, which has been caught for stopping */

  struct
  {
    DWORD time;    /* Total time usage in usecs */
    DWORD rss_mem; /* Total memory usage in Kbytes */
  } r_usage;


  char err_desc[RUN_ERR_DESC_LEN]; /* Description of occured error */
  GThread *thread; /* Pointer to thread */
} run_process_info_t;

/* Initialization of RUN STUFF */
int
run_init (void);

/* Uninitialization of RUN STUFF */
void
run_done (void);

/* Just create process info stucture for next using */
run_process_info_t*
run_create_process (const char *__cmd, const char *__workdir,
                    DWORD __rss_limit, DWORD __time_limit);

/* Free process information */
void
run_free_process (run_process_info_t *__self);

/* Execute process with profiling */
int
run_execute_process (run_process_info_t* __self);

/* Kill executed process */
int
run_kill_process (run_process_info_t* __self, int __state);

/* Wait for finishing of execution */
void
run_pwait (run_process_info_t* __self);

/* Set user and group from which processes qre starting */
void
run_set_usergroup (run_process_info_t* __self, long __uid, long __gid);

/* Set chroot's enabled */
void
run_set_chroot (run_process_info_t* __self, int __val);

/****
 * Internal used stuff
 */

/* Finalize process executing */
int
run_finalize_executing (run_process_info_t* __self);

/* Get process info by it's unique */
run_process_info_t*
run_process_info_by_unique (long __unique);

/* Handler for process has been terminated */
void
run_process_terminated (run_process_info_t* __self, int __sig);

/* Handler for process has been stopped */
void
run_process_stopped (run_process_info_t* __self, int __sig);

/* Handler for process has been coninued */
void
run_process_continued (run_process_info_t* __self);

/* Save base resources usage */
void
run_process_set_base (run_process_info_t* __self);

/* Overview belts of librun */
void
run_belts_overview (void);

END_HEADER

#endif
