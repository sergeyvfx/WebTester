/**
 * WebTester Server - server of on-line testing system
 *
 * Stuff for manipulating tasks for test
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_TASK_H_
#define _WT_TASK_H_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/dynastruc.h>
#include <libwebtester/assarr.h>
#include <libwebtester/mutex.h>

/***
 * Defenitions of different stuff
 */

#define MAXTASKBUF 4096
#define MAXCHAIN   32

/****
 * Task's ststus
 */
#define TS_PENDING      0x0000
#define TS_RUNNING      0x0001
#define TS_FINISHED     0x0002
#define TS_INTERRUPTED  0x0004

/****
 * Task's flags
 */
#define TF_UNPACKED    0x0001
#define TF_KEEPINQUEUE 0x0002

/****
 * Macroses
 */

#define TASK_STATUS(__self)              ((__self).status)
#define TASK_SET_STATUS(__self,__status) \
  { \
    TASK_STATUS(__self)=__status; \
    if (__status!=TS_RUNNING) \
      { \
        wt_task_set_belts_status_changed (); \
      } \
  }

#define TASK_LIBNAME(__self)       (wt_module_name ((__self).lid))

/****
 * Buffers
 */

/* General stuff */
#define TASK_LOCK_BUFFERS(__self)   g_mutex_lock   ((__self).buffers.lock)
#define TASK_UNLOCK_BUFFERS(__self) g_mutex_unlock ((__self).buffers.lock)

/* Pipe between module and CORE */
#define TASK_BUFFER(__self)       (__self).buffers.pipe
#define TASK_FREE_BUFFER(__self)  TASK_BUFFER_SET (__self, "")
#define TASK_BUFFER_SET(__self,__text) \
  { \
    TASK_LOCK_BUFFERS (__self); \
    strncpy ((__self).buffers.pipe, __text, MAXTASKBUF - 1); \
    TASK_UNLOCK_BUFFERS (__self); \
    if (strcmp (__text, "")) \
      { \
        wt_task_set_belts_status_changed (); \
      } \
  }
#define TASK_BUFFER_APPEND(__self,__text) \
  { \
    TASK_LOCK_BUFFERS (__self); \
    strncat ((__self).buffers.pipe, __text, \
             strlen ((__self).buffers.pipe) - MAXTASKBUF - 1); \
    TASK_UNLOCK_BUFFERS (__self); \
    wt_task_set_belts_status_changed (); \
  }
#define TASK_BUFFER_SET_LOCKED(__self,__text) \
    strncat ((__self).buffers.pipe, __text, \
             strlen ((__self).buffers.pipe) - MAXTASKBUF - 1);
#define TASK_FREE_BUFFER_LOCKED(__self) \
  TASK_BUFFER_SET_LOCKED (__self, "")

/* Resultation message */
#define TASK_RESULT_MESSAGE(__self)        ((__self).buffers.resultMessage)
#define TASK_FREE_RESULT_MESSAGE(__self)   TASK_SET_RESULT_MESSAGE (__self, "")
#define TASK_SET_RESULT_MESSAGE(__self,__text) \
  { \
    TASK_LOCK_BUFFERS (__self); \
    strncpy ((__self).buffers.resultMessage, __text, MAXTASKBUF - 1); \
    TASK_UNLOCK_BUFFERS (__self); \
    if (strcmp (__text, "")) \
      { \
        wt_task_set_belts_status_changed (); \
      } \
  }

/* Log */
#define TASK_GET_LOG(__self)         ((__self).buffers.log)
#define TASK_FREE_LOG(__self)        (TASK_SET_LOG (__self, ""))
#define TASK_SET_LOG(__self, __log) \
  { \
    TASK_LOCK_BUFFERS (__self); \
    (strncpy ((__self).buffers.log, __log, MAXTASKBUF -1)); \
    TASK_UNLOCK_BUFFERS (__self); \
  }

#define TASK_LOG(__self,__text,__args...) \
  { \
    char buf[65536]; \
    TASK_LOCK_BUFFERS (__self); \
    sprintf (buf, __text, ##__args); \
    strncat ((__self).buffers.log, buf, \
             strlen ((__self).buffers.log) - MAXTASKBUF - 1); \
    TASK_UNLOCK_BUFFERS (__self); \
  }

#define TASK_LOG_FLUSH(__self, __stream) \
  { \
    if (__stream) \
      { \
        TASK_LOCK_BUFFERS (__self); \
        fprintf (__stream, "%s", TASK_GET_LOG (__self)); \
        TASK_UNLOCK_BUFFERS (__self); \
      } \
  }

/* I/O parameters */
#define TASK_INPUT_PARAM(__self, __key) \
  (assarr_get_value ((__self).input_params, __key))

#define TASK_UNSET_OUTPUT_PARAM(__self, __key) \
  (assarr_unset_value ((__self).output_params, __key, \
                       assarr_deleter_free_ref_data))

#define TASK_SET_OUTPUT_PARAM(__self, __key, __val) \
  (assarr_set_value   ((__self).output_params, __key, __val))

/****
 * Task flags' stuff
 */

#define TASK_SET_FLAG(__self, __flag)    (__self).flags|=__flag
#define TASK_TEST_FLAG(__self, __flag)   ((__self).flags&__flag)
#define TASK_FREE_FLAG(__self, __flag)   (__self).flags&=~__flag

/****
 * Chain testing
 */

#define TASK_CHAINTEST(__self, __name) \
  wt_task_chaintest (__self, wt_module_id (__name))

#define TASK_CHAINEMPTY(__self) \
  (__self->chain_testing.cur >= (__self)->chain_testing.last)

#define TASK_CHAINLID(__self) \
  ((__self)->chain_testing.cur_lid)

#define TASK_CHAINSTARTED(__self) \
  ((__self)->chain_testing.started)

#define TASK_CURLID(__self) \
  (TASK_CHAINSTARTED (__self) ? TASK_CHAINLID (__self) : (__self)->lid)

/****
 * Type defenitions
 */

/* WebTester task */
typedef struct
{
  long sid; /* Solution id */
  int lid;  /* Library id */

  timeval_t timestamp; /* Timestamp of spawning task */

  int flags;  /* Different flags */
  int status; /* Status of task */

  struct
  {
    char pipe[MAXTASKBUF]; /* Pipe between module and CORE */
    char log[MAXTASKBUF];  /* Log of task's testing */
    char resultMessage[MAXTASKBUF]; /* Result message after testing finished */

    mutex_t lock;
  } buffers;

  assarr_t *input_params;  /* parameters, received by `get_task` */
  assarr_t *output_params; /* parameters to sent to webiface */

  /* Chain testing */
  struct
  {
    long length; /* Length of testing chain */
    int lid[MAXCHAIN]; /* Library IDs */
    int cur, last;     /* Pointer to current element in chain */
    int cur_lid;
    BOOL started;      /* Is chain testing started? */
  } chain_testing;
} wt_task_t;

/****
 * WebIFACE back-end
 */

/* Initialize task stuff */
int
wt_task_init (void);

/* Uninitialize task stuff */
void
wt_task_done (void);

/* Get list of untested tasks */
int
wt_get_task_list (dynastruc_t *__tasklist, int __queue_size);

/* Restore task's status */
int
wt_restore_task (wt_task_t *__self);

/* Get delailed info about task */
int
wt_get_task (wt_task_t *__self);

/* Put tested task to WebInterface */
int
wt_put_solution (wt_task_t *__self);

/* Reset status of half-tested tasks */
int
wt_reset_status (void);

/****
 * General stuff
 */

/* Spawn new task and pre-set fields */
wt_task_t*
wt_spawn_new_task (long __solution_id, int __library_id);

/* Free task info struct */
void
wt_task_free (wt_task_t *__self);

/* Set belts status changed */
void
wt_task_set_belts_status_changed (void);

/****
 * Dynastruc stuff
 */

/* Comparator for searching for task in dynastruc storage */
int
wt_task_search_comparator (void *__l, void *__r);

/* Dyna task deleter */
void
wt_task_deleter (void *__self);

/* Dyna task deleter with task restoring */
void
wt_task_deleter_with_restore (void *__self);

/****
 * Chain testing stuff
 */

/* Add library to chain */
int
wt_task_chaintest (wt_task_t *__self, int __lid);

END_HEADER

#endif
