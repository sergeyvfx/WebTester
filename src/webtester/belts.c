/*
 *
 * ================================================================================
 *  belts,h - part of the Webtester Server
 * ================================================================================
 *
 *  Belts' stuff
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "belts.h"
#include "queue.h"
#include "const.h"
#include "task.h"
#include "library.h"

#include <libwebtester/core.h>
#include <libwebtester/dynastruc.h>
#include <libwebtester/conf.h>
#include <libwebtester/log.h>

////
// Macroses

#define StateChanged stateChanged=TRUE

////
//

static dynastruc_t *belts = NULL;
static long belts_size    = BELTS_SIZE;
static BOOL global_state_changed = FALSE;

////
//

static void     // Read data from config file
read_config                        (void)
{
  CONFIG_INT_KEY (belts_size,  "Server/MainLoop/BeltsSize");

  // Some validations
  RESET_LEZ (belts_size,  BELTS_SIZE);
}

////
//

int             // Max avaliable size of belts
wt_belts_size                      (void)
{
  return belts_size;
}

int             // Init belts' stuff
wt_belts_init                      (void)
{
  read_config ();
  belts=dyna_create ();
  if (!belts) return -1;
  return 0;
}

void            // Free all cells of belts
wt_belts_free                      (void)
{
  dyna_delete_all (belts, wt_task_deleter_with_restore);
}

void            // Uninitialize belts stuff
wt_belts_done                      (void)
{
  //
  // Big troubles if belts is still busy and HTTP stuff
  // has been uninitialized
  //

  // dyna_destroy (belts, wt_task_deleter_with_restore);

  // But is it correct?
  dyna_destroy (belts, wt_task_deleter);
}

////////////////////////////////////////
// Belts' CORE builtin

static int      // Send task to lib for testing
send_for_testing                   (wt_task_t *__self, char *__error)
{
  strcpy (__error, "");
  DEBUG_LOG ("belts", "Seding task %ld@%d for testing...\n", __self->sid, __self->lid);
  if (wt_get_task (__self))
    {
      DEBUG_LOG ("belts", "Seding task %ld@%d for testing failed: could not get task's parameters\n", __self->sid, __self->lid);
      strcpy (__error, "Could not get task's parameters");
      return -1;
    }
  return wt_module_send_for_testing (__self, __error);
}

static void     // Return tested task to WebInterface
return_tested                      (wt_task_t *__self)
{
  core_print (MSG_INFO, "        Library %s finished testing task %ld@%d", TASK_LIBNAME (*__self), __self->sid, __self->lid);

  // Resultation message from testing library
  if (strcmp (TASK_RESULT_MESSAGE (*__self), ""))
    core_print (MSG_INFO, " (%s)", TASK_RESULT_MESSAGE (*__self));

  core_print (MSG_INFO, "\n");

  // Put tested task to WebInterface
  wt_put_solution (__self);
}

static BOOL     // Overview status of tasks in belts
belts_overview_status              (void)
{
  wt_task_t   *task;
  dyna_item_t *cur, *dummy;
  char        *pchar;
  BOOL stateChanged = FALSE;

  DEBUG_LOG ("belts", "Overviewing status...\n");

  cur=dyna_head (belts);
  while (cur)
    {
      task  = dyna_data (cur);
      dummy = cur;
      cur   = dyna_next (cur);

      // Message from testing module
      TASK_LOCK_BUFFERS (*task);
      pchar=TASK_BUFFER (*task);
      if (strcmp (pchar, ""))
        {
          core_print (MSG_INFO, "        %ld@%d: %s\n", task->sid, task->lid, pchar);

          //
          // TODO:
          //  Use forced freeing of buffer because buffers
          //  are already locked.
          //

          TASK_FREE_BUFFER_LOCKED (*task);
          StateChanged;
        }
      TASK_UNLOCK_BUFFERS (*task);

      // Testing of task is finished
      if (TASK_STATUS (*task)==TS_FINISHED) 
        {
          // Return tested task to client iface
          return_tested (task);
          // Free allocated memory for tested task and remove from belts
          dyna_delete (belts, dummy, wt_task_deleter);
          StateChanged;
        } else
      // Testing of stuff is interrupted
      if (TASK_STATUS (*task)==TS_INTERRUPTED) 
        {
          core_print (MSG_INFO, "        Restoring task %ld@%d (testing interrupted)\n", task->sid, task->lid);
          // Free allocated memory for
          dyna_delete (belts, dummy, wt_task_deleter_with_restore);
          StateChanged;
        }
    }

  return stateChanged;
}

static BOOL     // Fill belts from queue
belts_fill                         (dynastruc_t *__queue)
{
  char error[65536];
  wt_task_t   *task;
  dyna_item_t *cur;
  BOOL stateChanged = FALSE;

  DEBUG_LOG ("belts", "Filling belts...\n");

  cur=dyna_head (__queue);
  while (cur && !wt_belts_full ())
    {
      StateChanged;

      task=dyna_data (cur);
      dyna_delete (__queue, cur, 0); // Delete only node from queue
                                     // to free space for next tasks
      cur=dyna_next (cur);

      if (wt_task_in_belts (task))
        { 
          //
          // Task is already in belts.
          // But why LIB_WebIFACE hasn't cauched this?
          //

          //
          // Tasks are deleted from queue by default,
          // so just print warning
          //

          _WARNING ("        Duplicate caughting of task %ld@%d\n", task->sid, task->lid);
          continue;
        }

      // Free force keeping in belts flag
      TASK_FREE_FLAG (*task, TF_KEEPINQUEUE);

      if (!send_for_testing (task, error))
        {
          dyna_append (belts, task, 0);
          core_print (MSG_WARNING, "        Library %s started for testing task %ld@%d\n",
            TASK_LIBNAME (*task), task->sid, task->lid);
        } else
        {
          char pchar[65536];
          int type=MSG_WARNING;

          if (!strcmp (error, ""))
            strcpy (error, "<Unknown error>");

          // Prepare information string
          sprintf (pchar, "Unable to send task %ld@%d for testing: %s", task->sid, task->lid, error);

          if (TASK_TEST_FLAG (*task, TF_KEEPINQUEUE))
            {
              strcat (pchar, ". Not critical - continue waiting in queue.");
              type=MSG_INFO;
            }

          // Print da warning
          core_print (type, "        %s\n", pchar);

          if (!TASK_TEST_FLAG (*task, TF_KEEPINQUEUE))
            {
              // If not force keep in belts, restore task
              // in WebIFACE and 
              wt_restore_task (task);
              wt_task_free (task);
            } else
              // Returm task to queue
              dyna_push (__queue, task, 0);
        }
    }

  return stateChanged;
}

////////////////////////////////////////
//

BOOL            // Is task in belts?
wt_task_in_belts                   (wt_task_t *__self)
{
  dyna_search_reset (belts);
  return (dyna_search (belts, __self, 0, wt_task_search_comparator))!=0;
}

int             // Updating of belts
wt_belts_update                    (void)
{
  dynastruc_t *queue;
  int   belts_length, queue_length;
  BOOL stateChanged   = FALSE;
  static BOOL waiting = FALSE;

  queue=wt_queue ();

  queue_length=wt_queue_length ();
  belts_length=wt_belts_length ();

  // Some sucky beauty at output
  if (!global_state_changed && (!queue_length || belts_length>=belts_size))
    {
      if (!waiting && (queue_length || belts_length))
        {
          _INFO ("    Overviewing re-newed status...\n");
          _INFO ("        Waiting for status is changed...\n");
          _INFO ("    Overviewing completed\n");
        }
      waiting=TRUE;
      return 0;
    }
  waiting=FALSE;

  global_state_changed=FALSE;
  // Overview only it is posibility of some changes
  if (belts_length || (queue_length && belts_length<belts_size))
    {
      _INFO ("    Overviewing re-newed status...\n");

      // Look through cuttent belts and make some actions (delete if tested, start testing, etc..)
      stateChanged|=belts_overview_status ();

      // Fill belts with new tasks
      stateChanged|=belts_fill (queue);

      if (!stateChanged) core_print (MSG_INFO, "        Belts' state is unchanged\n");
      _INFO ("    Overviewing completed\n");
    }



  return 0;
}

BOOL            // Is belts empty?
wt_belts_empty                     (void)
{
  return dyna_empty (belts);
}

BOOL            // Is belts full?
wt_belts_full                      (void)
{
  return dyna_length (belts)>=belts_size;
}

long            // Current length of belts
wt_belts_length                    (void)
{
  return dyna_length (belts);
}

void
wt_belts_status_changed            (void)
{
  global_state_changed=TRUE;
}
