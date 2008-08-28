/*
 *
 * ================================================================================
 *  queue.c - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "autoinc.h"
#include "queue.h"
#include "task.h"

#include <libwebtester/dynastruc.h>

// Size of task queue
static long queue_size    = QUEUE_SIZE;
// Count of unpacking tasks per iteration
static long unpack_count  = UNPACK_COUNT;

// Task QUEUE
static dynastruc_t *queue = NULL;

////
//

static void     // Read data from config
read_config                        (void)
{
  CONFIG_INT_KEY   (queue_size,   "Server/MainLoop/QueueSize");
  CONFIG_INT_KEY   (unpack_count, "Server/MainLoop/UnpackCount");

  // Some validators
  RESET_LEZ (queue_size,   QUEUE_SIZE);
  RESET_LEZ (unpack_count, UNPACK_COUNT);
}

////////////////////////////////////////
//

int             // Initialize queue stuff
wt_queue_init                      (void)
{
  read_config ();
  queue=dyna_create ();
  if (!queue) return -1;
  return 0;
}

void            // Uninitialize queue stuff
wt_queue_done                      (void)
{
  dyna_destroy (queue, wt_task_deleter);
}

int
wt_queue_update                    (void)
{
  return wt_get_task_list (queue, queue_size);
}

void            // Unpack parameters for some tasks
wt_queue_unpack                    (void)
{
  wt_task_t *task;
  int got=0;

  DYNA_FOREACH (queue, task);
    if (got>=unpack_count) DYNA_BREAK;
    if (!TASK_TEST_FLAG (*task, TF_UNPACKED))
      {
        wt_get_task (task);
        got++;
      }
  DYNA_DONE ();
}

void            // Free all cells of queue
wt_queue_free                      (void)
{
  dyna_delete_all (queue, wt_task_deleter_with_restore);
}

long            // Size of queue
wt_queue_size                      (void)
{
  return queue_size;
}

long            // Current length oq queue
wt_queue_length                    (void)
{
  return dyna_length (queue);
}

BOOL            // Is queue empty?
wt_queue_empty                     (void)
{
  return dyna_empty (queue);
}

dynastruc_t*
wt_queue                           (void)
{
  return queue;
}
