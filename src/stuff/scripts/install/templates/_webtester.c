/*
 *
 * ================================================================================
 *  _webtester.c - part of the WebTester Server
 * ================================================================================
 *
 *  Supervisor of WebTester Server.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>

#include <string.h>

#define MAX_TRIES   10
#define TIME_DELTA  60

#define PIDFILE "/home/webtester/var/run/supervisor.pid"

////////
//

static void
write_pid                          (void)
{
  FILE *f=fopen (PIDFILE, "w");
  if (!f)
    return;

  fprintf (f, "%u", getpid ());

  fclose (f);
}

static void
rm_pid                             (void)
{
  unlink (PIDFILE);
}

static void
dirname                            (char *__self, char *__out)
{
  int i, n, last;

  n=strlen (__self);
  last=0;

  for (i=0; i<n; ++i)
    if (__self[i]=='/')
      last=i;

  for (i=0; i<last; ++i)
    __out[i]=__self[i];
  __out[i]=0;
}

static long
wt_get_uid                         (void)
{
  struct passwd *p=getpwnam ("webtester");
  if (!p) return -1;
  return (long)p->pw_uid;
}

static long
wt_get_gid                         (void)
{
  struct group *g=getgrnam ("webtester");
  if (!g) return -1;
  return (long)g->gr_gid;
}

static void
change_privilegies                 (void)
{
  long uid=wt_get_uid (),
       gid=wt_get_gid ();

  long ruid=getuid (), rgid=getgid ();

  if (uid<0 || gid<0)
    {
      fprintf (stderr, "Unable to determine webtester UID or GID\n");
      rm_pid ();
      exit (1);
    }

  if (ruid || rgid)
    {
      fprintf (stderr, "WebTester must be run by superuser\n");
      rm_pid ();
      exit (1);
    }

  if (setgroups (0, NULL)<0)
    {
      perror ("setgroups");
      rm_pid ();
      exit (1);
    }

  if (setregid (gid, gid)<0)
    {
      perror ("setregid");
      rm_pid ();
      exit (1);
    }

  if (setreuid (uid, uid)<0)
    {
      perror ("setreuid");
      rm_pid ();
      exit (1);
    }
}

////////
//

static void
signal_term                        (int __signum)
{
  int status;

  // Kill all childrens
  kill (0, __signum);

  // Wait for all childrens
  waitpid (-1, &status, WUNTRACED | WCONTINUED);

  rm_pid ();
  exit (0);
}

static void
hook_signals                       (void)
{
  signal (SIGINT,  signal_term);
  signal (SIGHUP,  signal_term);
  signal (SIGTERM, signal_term);
}

////////
//

static int      // Can we execute server?
executable                         (void)
{
  static int stack[MAX_TRIES+1];
  static int stack_ptr=0;
  
  int i, j, now=time (0);

  if (stack_ptr>=MAX_TRIES) return 0;

  i=0;
  while (i<stack_ptr)
    {
      if (now-stack[i]>TIME_DELTA)
        {
          for (j=i; j<stack_ptr-1; j++)
            stack[j]=stack[j+1];
          stack_ptr--;
        }
      i++;
    }

  stack[stack_ptr++]=now;
  return 1;
}

static void
exec_server                        (int __argc, char **__argv)
{
  static int initialized=0;
  static char *argv[4096];
  static char argv0[4096];

  if (!executable ())
    {
      // Oh, no! We can't execute server!
      fprintf (stderr, "WebTester server crashed %d times in last %d seconds. Not trying to restart.\n", MAX_TRIES, TIME_DELTA);
      rm_pid ();
      exit (1);
    }

  // Cacheable initialization of different stuff
  if (!initialized)
    {
      int i, j;
      char dummy[4096];

      j=1;
      for (i=1; i<__argc; i++)
        {
          if (strcmp (__argv[i], "--fork") && strcmp (__argv[i], "-f"))
            argv[j++]=__argv[i];
        }

      dirname (__argv[0], dummy);
      sprintf (argv0, "%s/bin/webtester.bin", dummy);
      argv[0]=argv0;

      argv[__argc] = 0;

      initialized=1;
    }

  // Fork and execute process
  if (fork ()==0)
    {
      execv (argv[0], argv);
      exit (errno);
    }
}

////////
// MAIN STUFF

int
main                               (int __argc, char **__argv)
{
  int status, w, i;
  int do_fork=0;

  for (i=1; i<__argc; i++)
    {
      if (!strcmp (__argv[i], "--fork") || !strcmp (__argv[i], "-f"))
        {
          do_fork=1;
        }
    }

  if (!do_fork || (fork()==0))
    {
      write_pid ();

      // Some initialization
      change_privilegies ();
      hook_signals ();

      for (;;)
        {
          exec_server (__argc, __argv);
          do
            {
              w=waitpid (-1, &status, WUNTRACED | WCONTINUED);
              if (w == -1) { rm_pid (); return -1; }
              if (WIFEXITED (status))
                {
                  int exit_code=WEXITSTATUS (status);
                  if (!exit_code)
                    {
                      rm_pid ();
                      return 0;
                    }
                }
              if (WIFSIGNALED (status))
                {
                  int termsig=WTERMSIG (status);
                  if (termsig==-1)
                    {
                      fprintf (stderr, "WebTester Server finished with exitcode -1. Assuming we don't need to restart server.\n");
                      rm_pid ();
                      return 0;
                    }
                  fprintf (stderr, "WebTester Server crashed!! Restarting...\n");
                  system ("sudo /home/webtester/sbin/lrvm_killall.sh");
                }
            }
          while (!WIFEXITED (status) && !WIFSIGNALED (status));
        }
      rm_pid ();
      return 0;
    }
  return 0;
}
