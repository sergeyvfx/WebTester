/*
 *
 * ================================================================================
 *  stat.h - part of the WebTester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "common.h"
#include "stat.h"
#include "builtin.h"
#include "iface.h"

#include <libwebtester/conf.h>
#include <libwebtester/sock.h>
#include <libwebtester/types.h>
#include <libwebtester/util.h>
#include <libwebtester/assarr.h>
#include <libwebtester/flexval.h>

#include <glib.h>

static double queue_pos=0, queue_delta=0.015;
static double belts_pos=0, belts_delta=0.03;

static GtkWidget *queue_progress=0;
static GtkWidget *belts_progress=0;

static GtkWidget *queue_usage=0;
static GtkWidget *belts_usage=0;

static BOOL queue_active = FALSE;
static BOOL belts_active = FALSE;

static int sock=-1;

static double full_update_interval=STAT_FULL_UPDATE_INTERVAL;

static GMutex *working    = NULL;

static GThread *update_thread    = NULL;
static GThread *listening_thread = NULL;

static assarr_t *vars_arr = NULL;
static assarr_t *stat_arr = NULL;

static int queue_size = 0, queue_used = 0;
static int belts_size = 0, belts_used = 0;

static double queue_cur_fraction=0, queue_prev_fraction=0;
static double belts_cur_fraction=0, belts_prev_fraction=0;

////////////////////////////////////////
// Internal stuff

static void
unpack_stat                        (char *__self)
{
//  printf ("%s\n", __self);
  assarr_unpack (__self+4, stat_arr);
  assarr_unpack (assarr_get_value (stat_arr, "vars"), vars_arr);
}

static void
parse_stat_arr                     (void)
{
  queue_active=is_truth (assarr_get_value (vars_arr, "Queue.Active"));
  belts_active=is_truth (assarr_get_value (vars_arr, "Belts.Active"));

  queue_size=flexval_atol (assarr_get_value (vars_arr, "Queue.Size"));
  belts_size=flexval_atol (assarr_get_value (vars_arr, "Belts.Size"));

  queue_used=flexval_atol (assarr_get_value (vars_arr, "Queue.Usage"));
  belts_used=flexval_atol (assarr_get_value (vars_arr, "Belts.Usage"));
}

static gpointer
update_thread_proc                 (gpointer __unused)
{
  struct timespec timestruc;
  timeval_t cur_time, last_update;
 
  timestruc.tv_sec   = 0;
  timestruc.tv_nsec  = 0.2*1000*1000*1000;

  cur_time=last_update=now ();

  for (;;)
    {
      if (g_mutex_trylock (working))
        {
          // For simple and correct uninitialisation stuff
          g_mutex_unlock (working);
          break;
        }

      if (sock>=0)
        if (CHECK_TIME_DELTA (last_update, cur_time, full_update_interval*USEC_COUNT))
          {
            if (sock_answer (sock, "stat packed\n"))
              disconnect_from_server ();
            last_update=cur_time;
          }

      nanosleep (&timestruc, 0);
      cur_time=now ();
    }

  return 0;
}

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

      if (sock>=0)
        {
          if (!read_ipc_sock (sock, &ans))
            {
              if (ans.type==IAT_OK)
                {
                  unpack_stat (IPC_ANS_BODY (ans));
                  parse_stat_arr ();
                }
            }
        }
      nanosleep (&timestruc, 0);
    }

  return 0;
}

static void
update_loop_progress               (GtkWidget *__widget, double *__pos, double *__delta)
{
  double a;
  if (__pos && __delta)
    {
      (*__pos)+=*__delta;

      if (*__pos>=1) (*__pos)=1, (*__delta)*=-1;
      if (*__pos<=0) (*__pos)=0, (*__delta)*=-1;
    }

  if (__pos && __widget)
    {
      a=*__pos;
//      gtk_progress_bar_pulse (__widget);
      gtk_progress_bar_set_fraction ((GtkProgressBar*)__widget, a);
    }
}

static void
preinitialize_iface                (void)
{
  iface_pipe_tab_hide ();
}

static void
read_config                        (void)
{
  CONFIG_FLOAT_KEY (full_update_interval, "Client/Stat/FullUpdateInterval");
}

////////////////////////////////////////
// User's backend

int
stat_init                          (void)
{
  queue_progress=lookup_widget (main_window, "queue_progress");
  belts_progress=lookup_widget (main_window, "belts_progress");

  queue_usage=lookup_widget (main_window, "queue_usage");
  belts_usage=lookup_widget (main_window, "belts_usage");

  read_config ();

//  gtk_progress_bar_set_pulse_step (queue_progress, 0.05);

  preinitialize_iface ();  

  working=g_mutex_new ();
  g_mutex_lock (working);

  vars_arr=assarr_create ();
  stat_arr=assarr_create ();

  update_thread=g_thread_create (update_thread_proc, 0, TRUE, 0);
  listening_thread=g_thread_create (listening_thread_proc, 0, TRUE, 0);

  return 0;
}

void
stat_done                          (void)
{
  g_mutex_unlock (working);
  g_thread_join (update_thread);
  g_thread_join (listening_thread);

  assarr_destroy (vars_arr, assarr_deleter_free_ref_data);
  assarr_destroy (stat_arr, assarr_deleter_free_ref_data);
}

static void
update_usage_progress              (GtkProgressBar *__self, int __used, int __size, double *__cur, double *__prev)
{
  double fraction;
  fraction=((__size)?((double)__used/__size):(0));

  if (fabs (fraction-*__prev)>0.03)
      *__prev=*__cur;

  if (fabs (fraction-*__cur)>0.03)
    *__cur+=0.03*sign (fraction-*__prev); else
    *__cur=fraction;

  if (*__cur<0) *__cur=0;
  if (*__cur>1) *__cur=1;

  gtk_progress_bar_set_fraction (__self, *__cur);
}

void
stat_update_monitors               (void)
{
  if (queue_active) update_loop_progress (queue_progress, &queue_pos, &queue_delta);
  if (belts_active) update_loop_progress (belts_progress, &belts_pos, &belts_delta);

  update_usage_progress ((GtkProgressBar*)queue_usage, queue_used, queue_size, &queue_cur_fraction, &queue_prev_fraction);
  update_usage_progress ((GtkProgressBar*)belts_usage, belts_used, belts_size, &belts_cur_fraction, &belts_prev_fraction);
}

void
stat_reset_monitors                (void)
{
  queue_pos=belts_pos=0;
  update_loop_progress (queue_progress, &queue_pos, &queue_delta);
  update_loop_progress (belts_progress, &belts_pos, &belts_delta);

  gtk_progress_bar_set_fraction ((GtkProgressBar*)queue_usage, 0);
  gtk_progress_bar_set_fraction ((GtkProgressBar*)belts_usage, 0);
}

void
stat_set_socket                    (int __self)
{
  ipc_ans_t ans;
  sock=__self;

  sock_answer (__self, "stat register\n");
  read_ipc_sock (__self, &ans);
}

int
stat_get_socket                    (void)
{
  return sock;
}

void
stat_on_disconnect                 (void)
{
  queue_active=belts_active=FALSE;
  queue_used=belts_used=queue_cur_fraction=belts_cur_fraction=0;
  stat_reset_monitors ();
}
