/**
 * WebTester Server - server of on-line testing system
 *
 * Belts' stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "belts.h"
#include "queue.h"
#include "const.h"
#include "task.h"
#include "library.h"
#include "stat.h"

#include <libwebtester/core.h>
#include <libwebtester/dynastruc.h>
#include <libwebtester/conf.h>
#include <libwebtester/log.h>
#include <libwebtester/ipc.h>
#include <libwebtester/scheduler.h>

#include <glib.h>

/****
 * Macroses
 */

#define StateChanged (stateChanged = TRUE)

static dynastruc_t *belts = NULL;
static dynastruc_t *accumulator = NULL;
static long belts_size = BELTS_SIZE;
static BOOL global_state_changed = FALSE;
static BOOL active = FALSE;
static __u64_t acc_overview_int = ACC_OVERVIEW_INT;
static GMutex *acc_mutex = NULL;

/* Handler for IPC connabd `belts` */
static int
ipc_belts (int __argc, char **__argv);

/**
 * Accumulator overview procidure
 */
static int
accumulator_overview_proc (void *__unused ATTR_UNUSED)
{
  dyna_item_t *cur, *dummy;
  wt_task_t *task;

  g_mutex_lock (acc_mutex);

  if (!accumulator)
    {
      g_mutex_unlock (acc_mutex);
      return 0;
    }

  core_print (MSG_INFO, "        Try to put solutions from accumulator...\n");

  cur = dyna_head (accumulator);
  while (cur)
    {
      dummy = cur;
      task = (wt_task_t*)dyna_data (cur);
      cur = dyna_next  (cur);

      if (!wt_put_solution (task))
        {
          core_print (MSG_INFO, "          Solution %ld@%d "
                                "successfully send.\n",
                      task->sid, task->lid);

          dyna_delete (accumulator, dummy, wt_task_deleter);
        }
      else
        {
          core_print (MSG_INFO, "          Solution %ld@%d "
                                "was unable to send.\n",
                      task->sid, task->lid);
        }
    }

  if (dyna_length (accumulator) == 0)
    {
      dyna_destroy (accumulator, NULL);
      accumulator = NULL;
    }

  g_mutex_unlock (acc_mutex);

  return 0;
}

/**
 * Accumulate task, which was tested,
 * but which was unable be send to WebIface
 *
 * @param __task - task to be accumulated
 */
static void
accumulate_solution (wt_task_t *__task)
{
  g_mutex_lock (acc_mutex);

  if (!accumulator)
    {
      /* Create deque for accumulator */
      accumulator = dyna_create ();

      scheduler_add (accumulator_overview_proc, 0, acc_overview_int);
    }

  dyna_add_to_back (accumulator, (void*)__task, 0);

  g_mutex_unlock (acc_mutex);
}

/**
 * Read data from config file
 */
static void
read_config (void)
{
  double dummy = 0;

  CONFIG_INT_KEY (belts_size, "Server/MainLoop/BeltsSize");

  CONFIG_FLOAT_KEY (dummy, "Server/MainLoop/AccumulatorOverviewInterval");

  if (dummy > 0)
    {
      acc_overview_int = dummy * USEC_COUNT;
    }

  /* Some validations */
  RESET_LEZ (belts_size, BELTS_SIZE);
}

/**
 * Send task to lib for testing
 *
 * @param __self - task to send
 * @param __error - buffer to store error description
 * @return zero on success, non-zero otjerwise
 */
static int
send_for_testing (wt_task_t *__self, char *__error)
{
  /* Free force keeping in belts flag */
  TASK_FREE_FLAG (*__self, TF_KEEPINQUEUE);

  strcpy (__error, "");

  DEBUG_LOG ("belts", "Seding task %ld@%d for testing...\n",
             __self->sid, __self->lid);

  if (wt_get_task (__self))
    {
      DEBUG_LOG ("belts", "Seding task %ld@%d for testing failed: "
                          "could not get task's parameters\n",
                 __self->sid, __self->lid);

      strcpy (__error, "Could not get task's parameters");

      return -1;
    }
  return wt_module_send_for_testing (__self, __error);
}

/**
 * Send task to chain module
 *
 * @param __self - task to be send
 * @param __error - error's description
 * @return zero on success, non-zero otherwise
 */
static int
send_to_chain (wt_task_t *__self, char *__error)
{
  __self->chain_testing.cur_lid =
    __self->chain_testing.lid[__self->chain_testing.cur];

  __self->chain_testing.started = TRUE;
  ++__self->chain_testing.cur;

  TASK_FREE_RESULT_MESSAGE (*__self);

  return wt_module_send_for_testing (__self, __error);
}

/**
 * Function to print message about module finished testing task
 *
 * @param __self - tested task
 */
static void
module_testing_finished (wt_task_t *__self)
{
  core_print (MSG_INFO, "        Library %s finished testing task %ld@%d",
              wt_module_name (TASK_CURLID (__self)), __self->sid, __self->lid);

  /* Resultation message from testing library */
  if (strcmp (TASK_RESULT_MESSAGE (*__self), ""))
    {
      core_print (MSG_INFO, " (%s)", TASK_RESULT_MESSAGE (*__self));
    }

  core_print (MSG_INFO, "\n");
}

/**
 * Return tested task to WebInterface
 *
 * @param __self - task to return
 */
static int
return_tested (wt_task_t *__self)
{
  /* Put tested task to WebInterface */
  if (wt_put_solution (__self))
    {
      /* Error sending tested task, so we need accumulate */
      /* this task in temporary storage to send it later */

      core_print (MSG_WARNING, "        Error in wt_put_solution(). "
                               "Put task %ld@%d to accumulator to "
                               "try send it later\n",
                  __self->sid, __self->lid);

     accumulate_solution (__self);

      return -1;
    }

  return 0;
}

/**
 * Overview status of tasks in belts
 *
 * @return is belts' status changed?
 */
static BOOL
belts_overview_status (void)
{
  wt_task_t *task;
  dyna_item_t *cur, *dummy;
  char *pchar;
  BOOL stateChanged = FALSE;

  DEBUG_LOG ("belts", "Overviewing status...\n");

  cur = dyna_head (belts);
  while (cur)
    {
      task = dyna_data (cur);
      dummy = cur;
      cur = dyna_next (cur);

      /* Message from testing module */
      TASK_LOCK_BUFFERS (*task);
      pchar = TASK_BUFFER (*task);
      if (strcmp (pchar, ""))
        {
          core_print (MSG_INFO, "        %ld@%d: %s\n",
                      task->sid, task->lid, pchar);

          /*
           * TODO: Use forced freeing of buffer because buffers
           *       are already locked.
           */

          TASK_FREE_BUFFER_LOCKED (*task);

          StateChanged;
        }
      TASK_UNLOCK_BUFFERS (*task);

      /* Testing of task is finished */
      if (TASK_STATUS (*task) == TS_FINISHED)
        {
          BOOL need_return = TRUE, work = TRUE;

          module_testing_finished (task);

          /* Try to send task to modules in chain */
          while (work && !TASK_CHAINEMPTY (task))
            {
              if (!send_to_chain (task, pchar))
                {
                  need_return = FALSE;
                  core_print (MSG_INFO, "        Chain-testing task %ld@%d"
                                        " on library %s\n",
                              task->sid, task->lid,
                              wt_module_name (TASK_CHAINLID (task)));
                  break;
                }
              else
                {
                  char msg[4096];
                  int type = MSG_WARNING;

                  snprintf (msg, BUF_SIZE (msg),
                            "        "
                            "Unable to send task %ld@%d for chain testing "
                            "to module %s: %s",
                            task->sid, task->lid,
                            wt_module_name (TASK_CHAINLID (task)), pchar);

                  if (TASK_TEST_FLAG (*task, TF_KEEPINQUEUE))
                    {
                      need_return = FALSE;
                      strcat (msg,
                              ". Not critical - continue waiting in chain.");
                      type = MSG_INFO;
                      --task->chain_testing.cur;
                      work = FALSE;
                    }
                  else
                    {
                      strcat (msg, ". Critical - skipping module in chain.");
                    }

                  core_print (type, "%s\n", msg);
                }
            }

          if (need_return)
            {
              /* Return tested task to client iface */
              if (!return_tested (task))
                {
                  /* Free allocated memory for tested task */
                  /* and remove from belts */
                  dyna_delete (belts, dummy, wt_task_deleter);
                }
              else
                {
                  /* Memory used by task will be freed as soon */
                  /* as task will be be send to WebIface from accumulator */
                  dyna_delete (belts, dummy, NULL);
                }
            }

          StateChanged;
        }
      else
        {
          /* Testing of stuff is interrupted */
          if (TASK_STATUS (*task) == TS_INTERRUPTED)
          {
            core_print (MSG_INFO, "        ""Restoring task %ld@%d "
                                  "(testing interrupted)\n",
                        task->sid, task->lid);

            /* Free allocated memory for */
            dyna_delete (belts, dummy, wt_task_deleter_with_restore);

            StateChanged;
          }
        }
    }

  return stateChanged;
}

/**
 * Fill belts from queue
 *
 * @return is belts' status changed?
 */
static BOOL
belts_fill (dynastruc_t *__queue)
{
  char error[4096];
  wt_task_t *task;
  dyna_item_t *cur;
  BOOL stateChanged = FALSE;
  char pchar[4096];

  DEBUG_LOG ("belts", "Filling belts...\n");

  cur = dyna_head (__queue);
  while (cur && !wt_belts_full ())
    {
      StateChanged;

      task = dyna_data (cur);

      /* Delete only node from queue */
      dyna_delete (__queue, cur, 0);

      /* to free space for next tasks */
      cur = dyna_next (cur);

      if (wt_task_in_belts (task))
        {
          /*
           * Task is already in belts.
           * But why LIB_WebIFACE hasn't cauched this?
           */

          /*
           * Tasks are deleted from queue by default,
           * so just print warning
           */

          _WARNING ("        Duplicate caughting of task %ld@%d\n",
                    task->sid, task->lid);

          wt_task_free (task);

          continue;
        }

      if (!send_for_testing (task, error))
        {
          dyna_append (belts, task, 0);
          core_print (MSG_WARNING, "        ""Library %s started for "
                                   "testing task %ld@%d\n",
                      TASK_LIBNAME (*task), task->sid, task->lid);
        }
      else
        {
          int type = MSG_WARNING;

          if (!strcmp (error, ""))
            {
              strcpy (error, "<Unknown error>");
            }

          /* Prepare information string */
          snprintf (pchar, BUF_SIZE (pchar),
                    "Unable to send task %ld@%d for testing: %s",
                    task->sid, task->lid, error);

          if (TASK_TEST_FLAG (*task, TF_KEEPINQUEUE))
            {
              strcat (pchar, ". Not critical - continue waiting in queue.");
              type = MSG_INFO;
            }

          /* Print da warning */
          core_print (type, "        %s\n", pchar);

          if (!TASK_TEST_FLAG (*task, TF_KEEPINQUEUE))
            {
              /* If not force keep in belts, restore task */
              /* in WebIFACE and */
              wt_restore_task (task);
              wt_task_free (task);
            }
          else
            {
              /* Returm task to queue */
              dyna_push (__queue, task, 0);
            }
        }
    }

  return stateChanged;
}

/********
 * User's backend
 */

/**
 * Initialize belts' stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
wt_belts_init (void)
{
  read_config ();
  belts = dyna_create ();
  if (!belts)
    {
      return -1;
    }

  ipc_proc_register ("belts", ipc_belts);

  wt_stat_set_int ("Belts.Size", belts_size);
  wt_stat_set_int ("Belts.Active", FALSE);

  acc_mutex = g_mutex_new ();

  return 0;
}

/**
 * Free all cells of belts
 */
void
wt_belts_free (void)
{
  dyna_delete_all (belts, wt_task_deleter_with_restore);
}

/**
 * Uninitialize belts stuff
 */
void
wt_belts_done (void)
{
  scheduler_remove (accumulator_overview_proc);

  g_mutex_lock (acc_mutex);
  if (accumulator)
    {
      dyna_destroy (accumulator, wt_task_deleter);
    }
  g_mutex_unlock (acc_mutex);
  g_mutex_free (acc_mutex);

  /*
   * Big troubles if belts is still busy and HTTP stuff
   * has been uninitialized
   */

  /* dyna_destroy (belts, wt_task_deleter_with_restore); */

  /* But is it correct? */
  dyna_destroy (belts, wt_task_deleter);
}

/**
 * Max avaliable size of belts
 *
 * @return max avaliable size of belts
 */
int
wt_belts_size (void)
{
  return belts_size;
}

/**
 * Is task in belts?
 *
 * @param __self - task to check
 * @return non-zero if task is already in belts
 */
BOOL
wt_task_in_belts (wt_task_t *__self)
{
  dyna_search_reset (belts);
  return (dyna_search (belts, __self, 0, wt_task_search_comparator)) != 0;
}

/**
 * Update belts
 *
 * @return zero on success, non-zero otherwise
 */
int
wt_belts_update (void)
{
  dynastruc_t *queue;
  int belts_length, queue_length;
  BOOL stateChanged = FALSE;
  static BOOL waiting = FALSE;
  static int update = TRUE;

  if (!active) /* Belts is not active */
    {
      return 0;
    }

  queue = wt_queue ();

  queue_length = wt_queue_length ();
  belts_length = wt_belts_length ();

  /* Some sucky beauty at output */
  if (!global_state_changed && (!queue_length || belts_length >= belts_size))
    {
      if (!waiting && (queue_length || belts_length))
        {
          _INFO ("    Overviewing re-newed status...\n");
          _INFO ("        Waiting for status is changed...\n");
          _INFO ("    Overviewing completed\n");
        }
      waiting = TRUE;

      if (update)
        {
          wt_stat_set_int ("Belts.Usage", wt_belts_length ());
          wt_stat_set_int ("Queue.Usage", wt_queue_length ());
        }

      update = FALSE;

      return 0;
    }

  waiting = FALSE;
  update = TRUE;

  global_state_changed = FALSE;

  /* Overview only it is posibility of some changes */
  if (belts_length || (queue_length && belts_length < belts_size))
    {
      _INFO ("    Overviewing re-newed status...\n");

      /* Look through cuttent belts and make some actions */
      /* (delete if tested, start testing, etc..) */
      stateChanged |= belts_overview_status ();

      /* Fill belts with new tasks */
      stateChanged |= belts_fill (queue);

      if (!stateChanged)
        {
          core_print (MSG_INFO, "        Belts' state is unchanged\n");
        }

      _INFO ("    Overviewing completed\n");
    }

  wt_stat_set_int ("Belts.Usage", wt_belts_length ());
  wt_stat_set_int ("Queue.Usage", wt_queue_length ());

  return 0;
}

/**
 * Is belts empty?
 *
 * @return non-zero if belts are empty, zero otherwise
 */
BOOL
wt_belts_empty (void)
{
  return dyna_empty (belts);
}

/**
 * Is belts full?
 *
 * @return non-zero if belts are full, zero otherwise
 */
BOOL
wt_belts_full (void)
{
  return dyna_length (belts) >= belts_size;
}

/**
 * Current length of belts
 *
 * @return length of belts
 */
long
wt_belts_length (void)
{
  return dyna_length (belts);
}

/**
 * Belts' status is changed
 */
void
wt_belts_status_changed (void)
{
  global_state_changed = TRUE;
}

/**
 * Start belts
 */
void
wt_belts_start (void)
{
  if (!active)
    {
      _INFO ("    Belts started\n");
      active = TRUE;
      wt_stat_set_int ("Belts.Active", TRUE);
    }
  else
    {
      _INFO ("    Belts is already started\n");
    }
}

/**
 * Stop belts
 */
void
wt_belts_stop (void)
{
  if (active)
    {
      _INFO ("    Belts stopped\n");
      active = FALSE;
      wt_stat_set_int ("Belts.Active", FALSE);
    }
  else
    {
      _INFO ("    Belts is already stopped\n");
    }
}

////////
// IPC builtin

/**
 * Start belts IPC command
 *
 * @param __argc - argument count
 * @param __argv - argument values
 * @return zero on success, non-zero otherwise
 */
static int
ipc_belts_start (int __argc, char **__argv)
{
  IPC_ADMIN_REQUIRED

  if (active)
    {
      IPC_PROC_ANSWER ("-ERR Belts is already started\n");
      return 0;
    }

  wt_belts_start ();
  IPC_PROC_ANSWER ("+OK Belts started\n");

  return 0;
}

/**
 * Stop belts IPC command
 *
 * @param __argc - argument count
 * @param __argv - argument values
 * @return zero on success, non-zero otherwise
 */
static int
ipc_belts_stop (int __argc, char **__argv)
{
  IPC_ADMIN_REQUIRED
  if (!active)
    {
      IPC_PROC_ANSWER ("-ERR Belts is already stopped\n");
      return 0;
    }

  wt_belts_stop ();

  IPC_PROC_ANSWER ("+OK Belts stopped\n");

  return 0;
}

/**
 * Belts IPC command
 *
 * @param __argc - argument count
 * @param __argv - argument values
 * @return zero on success, non-zero otherwise
 */
static int
ipc_belts (int __argc, char **__argv)
{
  if (__argc != 2)
    {
      goto __usage_;
    }

  if (!strcmp (__argv[1], "start"))
    {
      ipc_belts_start (__argc, __argv);
    }
  else
    {
      if (!strcmp (__argv[1], "stop"))
        {
          ipc_belts_stop (__argc, __argv);
        }
      else
        {
          goto __usage_;
        }
    }

  return 0;

__usage_:
  IPC_PROC_ANSWER ("-ERR: Usage: `belts [start|stop]`\n");
  return 0;
}
