/*
 *
 * ================================================================================
 *  pipe.c - part of the WebTester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "common.h"
#include "pipe.h"
#include "iface.h"
#include "builtin.h"
#include "console.h"

#include <libwebtester/sock.h>

#include <glib.h>

static int sock=-1;
static BOOL registered=FALSE;

static GMutex *working    = NULL;

static GThread *listening_thread = NULL;

////////
//

static void
update_widgets                     (void)
{
  if (registered)
    iface_pipe_tab_show (); else
    iface_pipe_tab_hide ();
}

////////
//

static gpointer
listening_thread_proc              (gpointer __unused)
{
  struct timespec timestruc;

  timestruc.tv_sec   = 0;
  timestruc.tv_nsec  = 0.2*1000*1000;

  ipc_ans_t ans;

  for (;;)
    {
      if (g_mutex_trylock (working))
        {
          // For simple and correct uninitialisation stuff
          g_mutex_unlock (working);
          break;
        }

      if (sock>=0 && registered)
        {
          if (!read_ipc_sock (sock, &ans))
            {
              if (ans.type==IAT_OK)
                {
                  log_to_widget ("pipe_view", "pipe_scroll", "%s", IPC_ANS_BODY (ans)+4);
                }
            }
        }

      nanosleep (&timestruc, 0);
    }

  return 0;
}


////////
//

BOOL
pipe_init                          (void)
{
  registered=FALSE;

  working=g_mutex_new ();
  g_mutex_lock (working);

  listening_thread=g_thread_create (listening_thread_proc, 0, TRUE, 0);

  return TRUE;
}

void
pipe_done                          (void)
{
  g_thread_join (listening_thread);
}

void
pipe_set_socket                    (int __self)
{
  sock=__self;

  if (__self<=0)
    {
      registered=FALSE;
    } else
    {
      ipc_ans_t ans;
      sock_answer (sock, "pipe register\n");
      read_ipc_sock (sock, &ans);  
      if (ans.type==IAT_OK)
        {
          registered=TRUE;
          log_to_widget ("pipe_view", "pipe_scroll", "Pipe from CORE registered.\nMessages before registering pipe isn't displayed.\n\n");
        }
    }

  update_widgets ();
}

int
pipe_get_socket                    (void)
{
  return sock;
}
