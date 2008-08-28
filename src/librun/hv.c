/*
 *
 * ================================================================================
 *  acct_hv.c - part of the LibRUN
 * ================================================================================
 *
 *  This module contains hypervisor under all accounting informatin,
 *  going from kernel space to user space.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <libwebtester/macrodef.h>
#include <libwebtester/core.h>
#include <libwebtester/conf.h>
#include <libwebtester/sock.h>
#include <libwebtester/log.h>

#include "run.h"
#include "hv.h"
#include "ipc.h"

#include <glib.h>

#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/genetlink.h>
#include <unistd.h>

////////
// Some constant defeninitons

#define MAX_CPU_MASK 128

//
// TODO:
//  We need both if this sockets to make synchranizing
//  stuff a bit easly
//

// Socket to get info about finished tasks
static int netlink_socket         = -1;
// Socet to info about specified task
static int netlink_socket_per_pid = -1;
// Identifier of netsock family id
static int family_id              = -1;
// Name of netlink family id
static char family[4096];

// ID of SELF
static pid_t mypid=0;

// Delay between two iteration of hypervisor cycles
static double hv_delay = RUN_HV_DELAY;

// Hypervisor main thread
static GThread *plistening_thread      = NULL;
static GThread *plistening_thread_acct = NULL;

// Mutexes. But does we need all of them?
static GMutex  *listening_thread_mutex = NULL;
static GMutex  *proc_stats_mutex       = NULL;
static GMutex  *send_cmd_mutex         = NULL;

// Mask of full CPU set
static char cpumask[MAX_CPU_MASK];

////////////////////////////////////////
// Deep internal STUFF

#define GENLMSG_DATA(__self) ((void*)(NLMSG_DATA(__self)+GENL_HDRLEN))
#define NLA_DATA(__self)     ((void*)((char*)(__self)+NLA_HDRLEN))
#define GENLMSG_PAYLOAD(glh) (NLMSG_PAYLOAD (glh, 0)-GENL_HDRLEN)
#define NLA_PAYLOAD(len)     (len - NLA_HDRLEN)

#define MAX_MSG_SIZE 4096

////////
// Type defenitions

struct msgtemplate {
	struct nlmsghdr n;
	struct genlmsghdr g;
	char buf[MAX_MSG_SIZE];
};

static int      // Create NetLINK socket
create_nl_socket                   (int __protocol, BOOL __nonblock)
{
  int nlsock;
  struct sockaddr_nl nladdr;

  nlsock=socket (AF_NETLINK, SOCK_RAW, __protocol);
  if (nlsock<0)
    return -1;

  sock_set_nonblock (nlsock, __nonblock);

  memset (&nladdr, 0, sizeof (nladdr));
  nladdr.nl_family=AF_NETLINK;

  if (bind (nlsock, (struct sockaddr*)&nladdr, sizeof(nladdr))<0)
    goto __fail_;

  return nlsock;
__fail_:
  close (nlsock);
  return -1;
}

static int      // Send command
send_cmd                           (int __sock, __u16 __nlmsg_type, __u32 __nlmsg_pid,
                                    __u8 __genl_cmd, __u16 __nla_type,
                                    void *__nla_data, int __nla_len)
{
  struct nlattr *na;
  struct sockaddr_nl nladdr;
  int r;
  int buflen;
  char *buf;

  struct msgtemplate msg;

  //
  // Debug code. We needn't lock mutex in all ways.
  // But maybe we need  to lock mutex per socket.
  //
  if (!send_cmd_mutex)
    return -1;
  g_mutex_lock (send_cmd_mutex);

  // Fill da msg struct
  msg.n.nlmsg_len   = NLMSG_LENGTH (GENL_HDRLEN);
  msg.n.nlmsg_type  = __nlmsg_type;
  msg.n.nlmsg_flags = NLM_F_REQUEST;
  msg.n.nlmsg_seq   = 0;
  msg.n.nlmsg_pid   = __nlmsg_pid;
  msg.g.cmd         = __genl_cmd;
  msg.g.version     = 0x1;

  na=(struct nlattr*)GENLMSG_DATA(&msg);
  na->nla_type=__nla_type;
  na->nla_len=__nla_len+1+NLA_HDRLEN;
  memcpy (NLA_DATA(na), __nla_data, __nla_len);
  msg.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

  buf=(char*)&msg;
  buflen=msg.n.nlmsg_len;

  memset (&nladdr, 0, sizeof (nladdr));
  nladdr.nl_family=AF_NETLINK;

  while ((r=sendto (__sock, buf, buflen, 0, (struct sockaddr*)&nladdr,
            sizeof (nladdr)))<buflen)
    {
      if (r>0)
        {
          buf+=r;
          buflen=r;
        } else
      if (errno!=EAGAIN)
        goto __fail_;
    }
  g_mutex_unlock (send_cmd_mutex);
  return 0;

__fail_:
  g_mutex_unlock (send_cmd_mutex);
  return -1;
}

static int      // Get family id for TASKSTATS family
get_family_id                      (int __sock, char *__name)
{
  struct msgtemplate ans;

  int id=0, rc;
  struct nlattr *na;
  int rep_len;

  //
  // TODO:
  // We needn't to lock mutexes here, because listening threads is not started yet,
  // and it wouldn't cause conflicts.
  //

  strcpy (__name, TASKSTATS_GENL_NAME);
  rc=send_cmd (__sock, GENL_ID_CTRL, getpid (), CTRL_CMD_GETFAMILY, CTRL_ATTR_FAMILY_NAME,
              (void*)__name, strlen (TASKSTATS_GENL_NAME)+1);

  rep_len=recv (__sock, &ans, sizeof (ans), 0);
  if (ans.n.nlmsg_type==NLMSG_ERROR || (rep_len<0) || !NLMSG_OK((&ans.n), rep_len))
    return 0;

  na=(struct nlattr*)GENLMSG_DATA(&ans);
  na=(struct nlattr*)((char*)na+NLA_ALIGN (na->nla_len));
  if (na->nla_type==CTRL_ATTR_FAMILY_ID)
    id=*(__u16*)NLA_DATA (na);
  return id;
}

////////////////
//

static void
get_real_cpu_mask                  (char *__out)
{
  FILE *p;
  int c, len=0;
  char script[4096]={0};

  strcpy (__out, cpumask);

  if (strcmp (cpumask, "auto"))
    return;

  // Automatize da deteting of CPU mask
  CONFIG_PCHAR_KEY (script, "CORE/CPUAutodetectScript");

  if (!strcmp (script, ""))
    return;
  
  p=popen (script, "r");
  
  if (!p)
    {
      _WARNING ("      LibRUN: error creating pipe to CPU autodetecting script. Use default CPU mask\n");
      return;
    }

  // Read result of working autodetection script
  while ((c=fgetc (p))!=EOF)
    __out[len++]=c;
  __out[len]=0;

  pclose (p);
}

static int      // Send query to kernel space
send_query_message                 (void)
{
  int rc;
  char cpumask[4096];

  get_real_cpu_mask (cpumask);

  core_print (MSG_INFO, "    **** CPU mask for LibRUN is %s\n", cpumask);

  rc=send_cmd (netlink_socket, family_id, mypid, TASKSTATS_CMD_GET,
    TASKSTATS_CMD_ATTR_REGISTER_CPUMASK,
    &cpumask, strlen (cpumask)+1);

  if (rc<0) return -1;
  return 0;
}

static gpointer// Review accounting informatin
listening_thread_acct              (gpointer __unused)
{
  int rep_len, len, len2, aggr_len;
  int rtid;
  struct msgtemplate msg;
  struct nlattr *na;

  for (;;)
    {

      //
      // TODO:
      //   Danger: Exiting from cycle will be only from executing some process
      //

      if (g_mutex_trylock (listening_thread_mutex))
        {
          // For simple and correct destroying
          g_mutex_unlock (listening_thread_mutex);
          break;
        }

      rep_len=recv (netlink_socket, &msg, sizeof(msg), 0);

      if (rep_len<0)
        {
          // Sock is blocking, Why this occured?
          continue;
        }

      if (msg.n.nlmsg_type==NLMSG_ERROR || !NLMSG_OK((&msg.n), rep_len))
        {
          struct nlmsgerr *err=NLMSG_DATA (&msg);
          core_print (MSG_WARNING, "librun: listening_thread_acct(): fatal reply error #%d\n", err->error);

          //
          // TODO:
          //  Does we need to do something here?
          //

          continue;
        }

      rep_len=GENLMSG_PAYLOAD (&msg.n);
      na=(struct nlattr*)GENLMSG_DATA (&msg);
      len=0;

      while (len<rep_len)
        {
          len+=NLA_ALIGN (na->nla_len);
          switch (na->nla_type)
            {
              case TASKSTATS_TYPE_AGGR_TGID:
              case TASKSTATS_TYPE_AGGR_PID:
                aggr_len=NLA_PAYLOAD (na->nla_len);
                len2=0;
                /* For nested attributes, na follows */
                na=(struct nlattr*)NLA_DATA (na);
                while (len2<aggr_len)
                  {
                    switch (na->nla_type)
                      {
                        case TASKSTATS_TYPE_PID:
                          rtid=*(int*)NLA_DATA (na);
                          break;
                        case TASKSTATS_TYPE_TGID:
                          rtid=*(int*)NLA_DATA (na);
                          break;
                        case TASKSTATS_TYPE_STATS:

                          //
                          // TODO:
                          //  Are we need collecting acc info only for tasks
                          //  with specified ppid (lrvm_pid) ?
                          //

                          run_hvpool_put ((struct taskstats*)NLA_DATA (na));
                          break;
                        default:
                          core_print (MSG_WARNING, "librun: listening_thread_acct(): unknown nla_type %d\n", na->nla_type);
                          break;
                      }
                    len2+=NLA_ALIGN (na->nla_len);
                    na=(struct nlattr*)((char*)na+len2);
                  }
                break;
              default:
                core_print (MSG_WARNING, "librun: listening_thread_acct(): unknown nla_type %d\n", na->nla_type);
                break;
            }
          na=(struct nlattr*)(GENLMSG_DATA (&msg)+len);
        }
 
      //
      // TODO:
      // Not use delaying because it causes some strange errors.
      //
    }

  g_thread_exit (0);
  return 0;
}

static gpointer
listening_thread                   (gpointer __unused)
{
  static struct timespec timestruc;
  timestruc.tv_sec  = ((long)(hv_delay*NSEC_COUNT))/NSEC_COUNT;
  timestruc.tv_nsec = ((long)(hv_delay*NSEC_COUNT))%NSEC_COUNT;

  for (;;)
    {
      if (g_mutex_trylock (listening_thread_mutex))
        {
          // For simple and correct destroying
          g_mutex_unlock (listening_thread_mutex);
          break;
        }

      run_ipc_listen ();
      run_ipc_interact ();
      run_belts_overview ();

      nanosleep (&timestruc, 0);
    }

  g_thread_exit (0);
  return 0;
}

////////////////////////////////////////
//

static BOOL
start_listening_thread             (void)
{
  listening_thread_mutex=g_mutex_new ();

  g_mutex_lock (listening_thread_mutex);

  plistening_thread      = g_thread_create (listening_thread,      0, TRUE, 0);
  plistening_thread_acct = g_thread_create (listening_thread_acct, 0, TRUE, 0);

  if (!plistening_thread || !plistening_thread_acct)
    return FALSE;

  return TRUE;
}

////////////////////////////////////////
// User-space stuff

static void     // Get info from config file
read_config                        (void)
{
  double t=0;

  // Delay between two iterations in listening thread
  CONFIG_FLOAT_KEY (t, "LibRUN/HyperVisor/Delay");
  if (t>0) hv_delay=t;

  // Mask of CPU set
  strcpy (cpumask, RUN_HV_CPUMASK);
  CONFIG_PCHAR_KEY (cpumask, "CORE/CPUMask");
}

int             // Initialization of HyoerVisor stuff
run_hypervisor_init                (void)
{
  read_config ();

  strcpy (family, "");
  mypid=getpid ();

  if (run_ipc_init ())
    return -1;

  if (run_hvpool_init ())
    return -1;

  // Create mutexes
  proc_stats_mutex=g_mutex_new ();
  send_cmd_mutex=g_mutex_new ();

  if ((netlink_socket         = create_nl_socket (NETLINK_GENERIC, FALSE))<0 ||
      (netlink_socket_per_pid = create_nl_socket (NETLINK_GENERIC, FALSE))<0
     )
    {
      core_set_last_error ("LibRUN: error while creating netlink sockets");
      core_print (MSG_WARNING,  "librun: error while creating netlink sockets");
      return -1;
    }

  // But is it different family ids for differ nt sockets?
  family_id=get_family_id (netlink_socket, family);

  if (send_query_message ()<0)
    {
      core_set_last_error ("LibRUN: error sending query message to kernel through netlink");
      core_print (MSG_WARNING,  "librun: error sending query message to kernel through netlink");
      return -1;
    }

  start_listening_thread ();
  return 0;
}

void            // Uninitializing of HyperVisor stuff
run_hypervisor_done                (void)
{
  if (listening_thread_mutex)
      g_mutex_unlock (listening_thread_mutex);

  // Stopping listening thread
  if (plistening_thread)
    g_thread_join (plistening_thread);

  if (plistening_thread_acct)
    g_thread_join (plistening_thread_acct);

  G_FREE_LOCKED_MUTEX (proc_stats_mutex);
  G_FREE_LOCKED_MUTEX (send_cmd_mutex);

  // Free mutex
  G_SAFE_FREE_MUTEX (listening_thread_mutex);

  // Close netlink sockets
  if (netlink_socket>=0)
    close (netlink_socket);

  if (netlink_socket_per_pid>=0)
    close (netlink_socket_per_pid);

  run_hvpool_done ();
  run_ipc_done ();
}

int             // ACCT info by process pid
run_hv_proc_stats                  (__u32 __pid, struct taskstats *__stats)
{
  int rc, rep_len, len, len2, aggr_len;
  int rtid;
  struct msgtemplate msg;
  struct nlattr *na;

  if (!proc_stats_mutex)
    return -1;

  DEBUG_LOG ("librun", "run_hv_proc_stats() called with pid %u\n", __pid);

  // Send request for info
  g_mutex_lock (proc_stats_mutex);

  rc=send_cmd (netlink_socket_per_pid, family_id, mypid, TASKSTATS_CMD_GET,
    TASKSTATS_CMD_ATTR_PID, &__pid, sizeof(__u32));

  if (rc<0)
    {
      core_print (MSG_WARNING,  "librun: run_hv_proc_stats(): error sending command to get stats structute by pid (%u)\n", __pid);
      g_mutex_unlock (proc_stats_mutex);
      return -1;
    }

  rep_len=recv (netlink_socket_per_pid, &msg, sizeof(msg), 0);
  g_mutex_unlock (proc_stats_mutex);

  if (rep_len<0)
    {
      core_print (MSG_WARNING,  "librun: run_hv_proc_stats(): error reading stats info for pid %ld\n", __pid);
      return -1;
    }

  if (msg.n.nlmsg_type==NLMSG_ERROR || !NLMSG_OK((&msg.n), rep_len))
    {
      // Some stuff is in `listening_thread_acct`, so the main reason of this
      // error is tasks's finito
      // core_print (MSG_WARNING,  "librun: run_hv_proc_stats(): unknown NLMSG type for pid %u\n", __pid);
      return -1;
    }

  rep_len=GENLMSG_PAYLOAD (&msg.n);

  if (!rep_len)
    {
      core_print (MSG_WARNING,  "librun: run_hv_proc_stats(): zero-length reply for pid %u\n", __pid);
    }

  na=(struct nlattr*)GENLMSG_DATA (&msg);
  len=0;

  while (len<rep_len)
    {
      len+=NLA_ALIGN (na->nla_len);
      switch (na->nla_type)
        {
          case TASKSTATS_TYPE_AGGR_TGID:
          case TASKSTATS_TYPE_AGGR_PID:
            aggr_len=NLA_PAYLOAD (na->nla_len);
            len2=0;
            /* For nested attributes, na follows */
            na=(struct nlattr*)NLA_DATA (na);
            while (len2<aggr_len)
              {
                switch (na->nla_type)
                  {
                    case TASKSTATS_TYPE_PID:
                      rtid=*(int*)NLA_DATA (na);
                      break;
                    case TASKSTATS_TYPE_TGID:
                      rtid=*(int*)NLA_DATA (na);
                      break;
                    case TASKSTATS_TYPE_STATS:
                      // Set out stats and exit
                      (*__stats)=*(struct taskstats*)NLA_DATA (na);
                      DEBUG_LOG ("librun", "run_hv_proc_stats(): collected data for pid %u\n", __pid);
                      return 0;
                      break;
                    default:
                      core_print (MSG_WARNING, "librun: run_hv_proc_stats(): unknown nla_type %d\n", na->nla_type);
                      return -1;
                      break;
                  }
                len2+=NLA_ALIGN (na->nla_len);
                na=(struct nlattr*)((char*)na+len2);
              }
            break;
          default:
            core_print (MSG_WARNING, "librun: run_hv_proc_stats(): unknown nla_type %d\n", na->nla_type);
            return -1;
            break;
        }
      na=(struct nlattr*)(GENLMSG_DATA (&msg)+len);
    }
  DEBUG_LOG ("librun", "run_hv_proc_stats(): error collecting data for pid %u\n", __pid);
  return -1;
}
