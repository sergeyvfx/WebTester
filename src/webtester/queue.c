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
#include "stat.h"

#include <libwebtester/dynastruc.h>
#include <libwebtester/ipc.h>

////////
//

// Size of task queue
static long queue_size    = QUEUE_SIZE;
// Count of unpacking tasks per iteration
static long unpack_count  = UNPACK_COUNT;

// Task QUEUE
static dynastruc_t *queue = NULL;

static BOOL active = FALSE;

////

static int
ipc_queue                          (int __argc, char **__argv);

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

  ipc_proc_register ("queue", ipc_queue);

  wt_stat_set_int ("Queue.Size",   queue_size);
  wt_stat_set_int ("Queue.Active", FALSE);

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
  int res;
  static int update=TRUE;
  if (!active) // Queue is not active
    {
      if (update)
        wt_stat_set_int ("Queue.Usage", wt_queue_length ());
      update=FALSE;
      return 0;
    }
  update=TRUE;
  res=wt_get_task_list (queue, queue_size);
  wt_stat_set_int ("Queue.Usage", wt_queue_length ());
  return res;
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
  DYNA_DONE;
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

void
wt_queue_start                     (void)
{
  if (!active)
    {
      _INFO ("    Queue started\n");
      active=TRUE;
      wt_stat_set_int ("Queue.Active", TRUE);
    } else
      _INFO ("    Queue is already started\n");
}

void
wt_queue_stop                      (void)
{
  if (active)
    {
      _INFO ("    Queue stopped\n");
      active=FALSE;
      wt_stat_set_int ("Queue.Active", FALSE);
    } else
      _INFO ("    Queue is already stopped\n");
}

////////
// IPC builtin

static int
ipc_queue_start                    (int __argc, char **__argv)
{
  IPC_ADMIN_REQUIRED
  if (active)
    { 
      IPC_PROC_ANSWER ("-ERR Queue is already started\n");
      return 0;
    }

  wt_queue_start ();
  IPC_PROC_ANSWER ("+OK Queue started\n");

  return 0;
}

static int
ipc_queue_stop                     (int __argc, char **__argv)
{
  IPC_ADMIN_REQUIRED
  if (!active)
    { 
      IPC_PROC_ANSWER ("-ERR Queue is already stopped\n");
      return 0;
    }

  wt_queue_stop ();
  IPC_PROC_ANSWER ("+OK Queue stopped \n");

  return 0;
}

static int
ipc_queue                          (int __argc, char **__argv)
{
  if (__argc!=2)
    goto __usage_;

  if (!strcmp (__argv[1], "start"))
    {
      ipc_queue_start (__argc, __argv);
    } else
  if (!strcmp (__argv[1], "stop"))
    {
      ipc_queue_stop (__argc, __argv);
    } else
      goto __usage_;

  return 0;
__usage_:
  IPC_PROC_ANSWER ("-ERR: Usage: `queue [start|stop]`\n");
  return 0;
}
