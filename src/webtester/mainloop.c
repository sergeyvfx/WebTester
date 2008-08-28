/*
 *
 * ================================================================================
 *  mainloop.c - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "autoinc.h"
#include "mainloop.h"
#include "queue.h"
#include "belts.h"

#include <libwebtester/flexval.h>
#include <libwebtester/hook.h>
#include <libwebtester/dynastruc.h>
#include <libwebtester/util.h>
#include <libwebtester/scheduler.h>

#include <glib.h>
#include <time.h>

////////
// MACRO defenitions

#define CALL_DELAYED(__last_timestamp, __now, __time_delta, __proc, __suff) \
  if (CHECK_TIME_DELTA (__last_timestamp, __now, __time_delta)) \
    { \
      __proc (); \
      __last_timestamp=__now; \
      __suff; \
    }

////////
// Queue and belts update interval
static double queue_update_interval = MAINLOOP_QUEUE_UPDATE_INTERVAL;
static double belts_update_interval = MAINLOOP_BELTS_UPDATE_INTERVAL;
// Task upload interval
static double upload_interval       = MAINLOOP_UPLOAD_INTERVAL;
// Param. unpack interval
static double unpack_interval       = MAINLOOP_UNPACK_INTERVAL;
// Scheduler overview interval
static double scheduler_interval    = SCHEDULER_INTERVAL;

// Mainloop thread
static GThread *mainloop  = NULL;
static GMutex *working    = NULL;

// Delay in main loop
static double delay=MAINLOOP_DELAY;

static BOOL queue_autostart = QUEUE_AUTOSTART;
static BOOL belts_autostart = BELTS_AUTOSTART;

////
// Some prototypes

static gpointer // Main loop
wt_mainloop                        (gpointer __unused);

////
// Some stupid stuff

static void     // Read data from config
read_config                        (void)
{
  CONFIG_FLOAT_KEY (queue_update_interval, "Server/MainLoop/Queue/UpdateInterval");
  CONFIG_FLOAT_KEY (queue_autostart,       "Server/MainLoop/Queue/Autostart");
  CONFIG_FLOAT_KEY (belts_update_interval, "Server/MainLoop/Belts/UpdateInterval");
  CONFIG_FLOAT_KEY (belts_autostart,       "Server/MainLoop/Belts/Autostart");
  CONFIG_FLOAT_KEY (upload_interval,       "Server/MainLoop/UpdateInterval");
  CONFIG_FLOAT_KEY (unpack_interval,       "Server/MainLoop/UnpackInterval");
  CONFIG_FLOAT_KEY (delay,                 "Server/MainLoop/Delay");
  CONFIG_FLOAT_KEY (scheduler_interval,    "Server/Scheduler/OverviewInterval");


  // Some validators
  RESET_LEZ (queue_update_interval, MAINLOOP_QUEUE_UPDATE_INTERVAL);
  RESET_LEZ (belts_update_interval, MAINLOOP_BELTS_UPDATE_INTERVAL);
  RESET_LEZ (upload_interval,       MAINLOOP_UPLOAD_INTERVAL);
  RESET_LEZ (unpack_interval,       MAINLOOP_UNPACK_INTERVAL);
  RESET_LEZ (unpack_interval,       MAINLOOP_UNPACK_INTERVAL);
  RESET_LEZ (delay,                 MAINLOOP_DELAY);
  RESET_LEZ (scheduler_interval,    SCHEDULER_INTERVAL);
}

////////////////////////////////////////
//

static int      // Start testing main loop
wt_mainloop_start                  (void *__unused, void *__call_unused)
{
  // Reset status of half-tested tasks
  wt_reset_status ();
  core_print (MSG_INFO, "    **** Testing main loop started. Queue size: %d, Belts size: %d.\n", wt_queue_size (), wt_belts_size ());
  g_mutex_lock (working);
  mainloop=g_thread_create (wt_mainloop, 0, TRUE, 0);
  return 0;
}

static int      // Stop testing mainloop
wt_mainloop_stop                   (void *__unused, void *__call_unused)
{
  if (g_mutex_trylock (working))
    {
      // Main loop is not running
      g_mutex_unlock (working);
      return 0;
    }

  _INFO ("    Sending StopTesting signal to all modules...");
  g_mutex_unlock (working);
  hook_call (CORE_STOPTESTING, 0);
  g_thread_join (mainloop);
  CMSG_DONE ();

  // Free queue and belts
  if (wt_queue_length () || wt_belts_length ())
    {
      core_print (MSG_INFO, "    Restoring tasks from queue and belts...");
      wt_queue_free ();
      wt_belts_free ();
      CMSG_DONE ();
    }

  core_print (MSG_INFO, "    **** Testing main loop deactivated.\n");
  return 0;
}

////////////////////////////////////////
// Main stuff

int             // Initialization of testing mainloop
wt_mainloop_init                   (void)
{
  // Recieve some settings
  read_config ();

  // Register hooks
  hook_register (CORE_ACTIVATE,   wt_mainloop_start, 0, HOOK_PRIORITY_NORMAL);
  hook_register (CORE_DEACTIVATE, wt_mainloop_stop,  0, HOOK_PRIORITY_NORMAL);

  if (wt_task_init ())
    return -1;

  wt_queue_init ();
  wt_belts_init ();

  working=g_mutex_new ();

  return 0;
}

void            // Uninitialize testing mainloop
wt_mainloop_done                   (void)
{
  wt_mainloop_stop (0, 0);

  wt_queue_done ();
  wt_belts_done ();

  if (working)
    g_mutex_free (working);

  wt_task_done ();
}

////////////////////////////////////////
// Testing main loop

static void
call_upload_tasks                  (void)
{
  hook_call (CORE_UPLOADPROBLEMS, 0); // Call tasks upload hook
}

static gpointer // Main loop
wt_mainloop                        (gpointer __unused)
{
  struct timespec timestruc;
  timeval_t last_queue_update, last_belts_update, cur_time, last_upload, last_unpack, last_scheduler;
  double    queue_delta     = queue_update_interval*USEC_COUNT; // Microseconds
  double    belts_delta     = belts_update_interval*USEC_COUNT; // Microseconds
  double    upload_delta    = upload_interval*USEC_COUNT;
  double    unpack_delta    = unpack_interval*USEC_COUNT;
  double    scheduler_delta = scheduler_interval*USEC_COUNT;
  BOOL      waiting, check_state;

  last_queue_update = last_belts_update = last_scheduler = last_unpack = last_upload = cur_time = now ();

  timestruc.tv_sec   = ((unsigned long long)(delay*NSEC_COUNT))/NSEC_COUNT;
  timestruc.tv_nsec  = ((unsigned long long)(delay*NSEC_COUNT))%NSEC_COUNT;

  nanosleep (&timestruc, 0);

  if (queue_autostart)  wt_queue_start ();
  if (belts_autostart)  wt_belts_start ();

  core_print (MSG_INFO, "    Waiting for tasks...\n");
  waiting=TRUE;

  for (;;)
    {
      check_state=FALSE;

      if (g_mutex_trylock (working))
        {
          // For simple and correct uninitialisation stuff
          g_mutex_unlock (working);
          break;
        }

      // Overview scheduler status
      CALL_DELAYED (last_scheduler, cur_time, scheduler_delta, scheduler_overview, 0);

      // Unpack params of some tasks for some optimization
      CALL_DELAYED (last_unpack, cur_time, unpack_delta, wt_queue_unpack, 0);

      // Update queue
      CALL_DELAYED (last_queue_update, cur_time, queue_delta, wt_queue_update, check_state|=TRUE);
      // Update belts
      CALL_DELAYED (last_belts_update, cur_time, belts_delta, wt_belts_update, check_state|=TRUE);

      // Do update stuff here
      if (check_state)
        {
          if (wt_belts_empty () && wt_queue_empty ())
            {
              // For some sucky beauty
              if (!waiting)
                {
                  core_print (MSG_INFO, "    Nothing to do. Waiting for tasks...\n");
                  waiting=TRUE;
                }
            } else 
              waiting=FALSE;
        }

      CALL_DELAYED (last_upload, cur_time, upload_delta, call_upload_tasks, 0);

      nanosleep (&timestruc, 0);
      cur_time=now ();
    }
//  g_thread_exit (0);
  return 0;
}
