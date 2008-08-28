/*
 *
 * ================================================================================
 *  task.h
 * ================================================================================
 *
 *  Stuff for manipulating tasks for test
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_task_h_
#define _wt_task_h_

#include <libwebtester/smartinclude.h>
#include <libwebtester/dynastruc.h>
#include <libwebtester/assarr.h>
#include <libwebtester/mutex.h>

#ifndef _wt_belts_h_
void wt_belts_status_changed (void);
#endif

////////////////////////////////////////
// Defenitions of different stuff

#define MAXTASKBUF 65536

////////
// Task's ststus
#define TS_PENDING      0x0000
#define TS_RUNNING      0x0001
#define TS_FINISHED     0x0002
#define TS_INTERRUPTED  0x0004

////////
// Task's flags
#define TF_UNPACKED    0x0001
#define TF_KEEPINQUEUE 0x0002

////////////////////////////////////////
// Macroses

#define TASK_STATUS(__self)              ((__self).status)
#define TASK_SET_STATUS(__self,__status) \
  { \
    TASK_STATUS(__self)=__status; \
    if (__status!=TS_RUNNING) \
      wt_belts_status_changed (); \
  }

#define TASK_LIBNAME(__self)       (wt_module_name ((__self).lid))

////////
// Buffers

// General stuff

#define TASK_LOCK_BUFFERS(__self)   g_mutex_lock   ((__self).buffers.lock)
#define TASK_UNLOCK_BUFFERS(__self) g_mutex_unlock ((__self).buffers.lock)

// Pipe between module and CORE
#define TASK_BUFFER(__self)       (__self).buffers.pipe
#define TASK_FREE_BUFFER(__self)  TASK_BUFFER_SET (__self, "")
#define TASK_BUFFER_SET(__self,__text) \
  { \
    TASK_LOCK_BUFFERS (__self); \
    strcpy ((__self).buffers.pipe, __text); \
    TASK_UNLOCK_BUFFERS (__self); \
    if (strcmp (__text, "")) \
      wt_belts_status_changed (); \
  }
#define TASK_BUFFER_APPEND(__self,__text) \
  { \
    TASK_LOCK_BUFFERS (__self); \
    strcat ((__self).buffers.pipe, __text); \
    TASK_UNLOCK_BUFFERS (__self); \
    wt_belts_status_changed (); \
  }
#define TASK_BUFFER_SET_LOCKED(__self,__text) \
    strcat ((__self).buffers.pipe, __text);
#define TASK_FREE_BUFFER_LOCKED(__self) \
  TASK_BUFFER_SET_LOCKED (__self, "")

// Resultation message
#define TASK_RESULT_MESSAGE(__self)            ((__self).buffers.resultMessage)
#define TASK_FREE_RESULT_MESSAGE(__self)       TASK_SET_RESULT_MESSAGE (__self, "")
#define TASK_SET_RESULT_MESSAGE(__self,__text) \
  { \
    TASK_LOCK_BUFFERS (__self); \
    strcpy ((__self).buffers.resultMessage, __text); \
    TASK_UNLOCK_BUFFERS (__self); \
    if (strcmp (__text, "")) \
      wt_belts_status_changed (); \
  }

// Log
#define TASK_GET_LOG(__self)         ((__self).buffers.log)
#define TASK_FREE_LOG(__self)        (TASK_SET_LOG (__self, ""))
#define TASK_SET_LOG(__self, __log) \
  { \
    TASK_LOCK_BUFFERS (__self); \
    (strcpy ((__self).buffers.log, __log)); \
    TASK_UNLOCK_BUFFERS (__self); \
  }

#define TASK_LOG(__self,__text,__args...) \
  { \
    char buf[65536]; \
    TASK_LOCK_BUFFERS (__self); \
    sprintf (buf, __text, ##__args); \
    strcat ((__self).buffers.log, buf); \
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

// I/O parameters

#define TASK_INPUT_PARAM(__self, __key) (assarr_get_value ((__self).input_params, __key))
#define TASK_UNSET_OUTPUT_PARAM(__self, __key)      (assarr_unset_value ((__self).output_params, __key, assarr_deleter_free_ref_data))
#define TASK_SET_OUTPUT_PARAM(__self, __key, __val) (assarr_set_value   ((__self).output_params, __key, __val))

////
// Task flags' stuff

#define TASK_SET_FLAG(__self, __flag)    (__self).flags|=__flag
#define TASK_TEST_FLAG(__self, __flag)   ((__self).flags&__flag)
#define TASK_FREE_FLAG(__self, __flag)   (__self).flags&=~__flag

////////////////////////////////////////
// Type defenitions

// WebTester task
typedef struct {
  long sid;       // Solution id
  int  lid;       // Library id

  timeval_t timestamp; // Timestamp of spawning task

  int  flags;     // Different flags
  int  status;    // Status of task

  struct {
    char pipe[MAXTASKBUF];          // Pipe between module and CORE
    char log[MAXTASKBUF];           // Log of task's testing
    char resultMessage[MAXTASKBUF]; // Result message after testing finished

    mutex_t lock;
  } buffers;

  assarr_t *input_params;  // parameters, received by `get_task`
  assarr_t *output_params; // parameters to sent to webiface
} wt_task_t;

////////////////////////////////////////
// WebIFACE back-end

int             // Initialize task stuff
wt_task_init                       (void);

void            // Uninitialize task stuff
wt_task_done                       (void);

int             // Get list of untested tasks
wt_get_task_list                   (dynastruc_t *__tasklist, int __queue_size);

int             // Restore task's status
wt_restore_task                    (wt_task_t *__self);

int             // Get delailed info about task
wt_get_task                        (wt_task_t *__self);

int             // Put tested task to WebInterface
wt_put_solution                    (wt_task_t *__self);

int             // Reset status of half-tested tasks
wt_reset_status                    (void);

////////////////////////////////////////
// General stuff

wt_task_t*      // Spawn new task and pre-set fields
wt_spawn_new_task                 (long __solution_id, int __library_id);

void            // Free task info struct
wt_task_free                      (wt_task_t *__self);

////////
// Dynastruc stuff

int             // Comparator for searching for task in dynastruc storage
wt_task_search_comparator         (void *__l, void *__r);

void
wt_task_deleter                   (void *__self);

void
wt_task_deleter_with_restore      (void *__self);

#endif
