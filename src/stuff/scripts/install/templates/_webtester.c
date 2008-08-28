/*
 *
 * ================================================================================
 *  _webtester.c - part of the WebTester Server
 * ================================================================================
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

#define MAX_TRIES   10
#define TIME_DELTA  60

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
      printf ("Unable to determine webtester UID or GID\n");
      exit (1);
    }

  if (setgroups (0, NULL)<0)
    {
      perror ("setgroups");
      exit (1);
    }

  if (setregid (gid, gid)<0)
    {
      perror ("setregid");
      exit (1);
    }

  if (setreuid (uid, uid)<0)
    {
      perror ("setreuid");
      exit (1);
    }
}

static void
signal_term                        (int __signum)
{
}

static void
hook_signals                       (void)
{
  signal (SIGINT,  signal_term);
  signal (SIGHUP,  signal_term);
  signal (SIGTERM, signal_term);
}

static int
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

  if (!executable ())
    {
      fprintf (stderr, "WebTester server crashed %d times in last %d seconds. Not trying to restart.\n", MAX_TRIES, TIME_DELTA);
      exit (1);
    }

  if (!initialized)
    {
      int i;
      for (i=1; i<__argc; i++)
        argv[i]=__argv[i];
      argv[0]      = "./bin/webtester.bin";
      argv[__argc] = 0;

      initialized=1;
    }

  if (fork ()==0)
    {
      execv (argv[0], argv);
      exit (errno);
    }
}

int
main                               (int __argc, char **__argv)
{
  int status, w;

  change_privilegies ();
  hook_signals ();

  for (;;)
    {
      exec_server (__argc, __argv);
      do
        {
          waitpid (-1, &status, WUNTRACED | WCONTINUED);
          if (w == -1) { return -1; }
          if (WIFEXITED (status))
            {
              int exit_code=WEXITSTATUS (status);
              if (!exit_code)
                return 0;
            }
          if (WIFSIGNALED (status))
            {
              int termsig=WTERMSIG (status);
              fprintf (stderr, "Webtester Server crashed!! Restarting...\n");
              system ("sudo /home/webtester/sbin/lrvm_killall.sh");
            }
        }
      while (!WIFEXITED (status) && !WIFSIGNALED (status));
    }

  return 0;
}
