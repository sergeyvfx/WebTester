/**
 * WebTester Server - server of on-line testing system
 *
 * Part of the lrvm - client part of LibRUN
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */


#include <libwebtester/smartinclude.h>
#include <libwebtester/sock.h>
#include <libwebtester/cmd.h>
#include <libwebtester/md5.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
#include <grp.h>

#include <errno.h>

#define CONFIG_FILE            "/etc/lrvm.conf"

#define SECURITY_MAGICK        "asdadfad"

#define DEFAULT_READ_TIMEOUT   (2.5*NSEC_COUNT)
#define DEFAULT_READ_DELAY     (0.02*NSEC_COUNT)

#define C_ARG_EQ(__s) (!strcmp (__argv[i], __s))

static int sock = -1;
static DWORD read_timeout = DEFAULT_READ_TIMEOUT;
static DWORD read_delay = DEFAULT_READ_DELAY;

static int exiting = 0;
static char config_file[4096];

static char *config_allowed_prefixes[] = {
  "/etc",
  HOME_DIRECTORY,
  0
};

static int allow_uid = 0, allow_gid = 0;

/****
 * Some forward defenitions
 */

/* Terminate working of all stuff */
static void
term (void);

/* Send  command throug IPC interface */
static void
lrvm_ipc_send_command (char *__command, char *__param);

/****
 * IPC stuff
 */

/**
 * Wait answer from socket
 *
 * @param __maxlen - max buffer length
 * @param __buf - buffer to store answer
 */
static void
lrvm_ipc_wait_answer (int __maxlen, char *__buf)
{
  struct timespec timestruc;
  int trycount = read_timeout / read_delay;

  if (trycount <= 0)
    {
      trycount = 1;
    }

  timestruc.tv_sec = read_delay / NSEC_COUNT;
  timestruc.tv_nsec = read_delay % NSEC_COUNT;

  for (;;)
    {
      if (sock_read (sock, __maxlen, __buf) > 0)
        break;
      trycount--;
      if (!trycount)
        {
          /* No answer for too long. Terminate executing. */
          term ();
        }
      nanosleep (&timestruc, 0);
    }

  if (!strncmp (__buf, "-ERR", 4))
    {
      /* Got an error */
      if (!exiting)
        {
          /*
           * If not already exiting, try to send bug report to
           * server stuff
           */
          char buf[1024];
          int i, n;

          exiting = 1;
          n = strlen (__buf);
          strcpy (buf, "");

          for (i = 0; i < n - 4; i++)
            {
              buf[i] = __buf[i + 4];
            }
          buf[i] = 0;

          lrvm_ipc_send_command ("exec_error", buf);
        }
      term ();
    }
}

/**
 * Initialization if IPC stuff
 *
 * @param __hostname - server hostname
 * @param __port - server port
 * @return socket to use
 */
static int
lrvm_ipc_init (const char *__hostname, long __port)
{
  char buf[128];
  sock = sock_create_client_inet (__hostname, __port);
  if (sock < 0)
    {
      term ();
    }
  sock_set_nonblock (sock, TRUE);
  lrvm_ipc_wait_answer (127, buf);
  if (strncmp (buf, "+OK", 3))
    {
      /* If answer not equals to +OK, smth strange has been */
      /* happened, so terminate */
      term ();
    }
  return sock;
}

/**
 * Uninitialize IPC interface
 */
static void
lrvm_ipc_done (void)
{
  sock_destroy (sock);
}

/**
 * Send IPC command
 */
static void
lrvm_ipc_send_command (char *__command, char *__param)
{
  static char buf[1024];
  if (!sock_answer (sock, "%s %s\n", __command, __param))
    {
      /* Do not wait answer if finalizing */
      //* f (strcmp (__command, "finalize")) */
      lrvm_ipc_wait_answer (1024, buf);
    }
  else
    {
      /* Some error during sending command */
      term ();
    }
}

static void
term (void)
{
  lrvm_ipc_done ();

  /* Kill all childs */
  kill (-getpid (), SIG_KILL);

  /* Kill self */
  kill (getpid (), SIG_TERM);

  /* Exit. But does we need this? */
  exit (-1);
}

/****
 * MAIN STUFF
 */

/**
 * Validate sequrity hashes
 *
 * @param __s - hash to validate to
 * @param __unique - unique
 */
static void
validate_security (char *__s, long __unique)
{
  char tmp[128], dummy[128], key[128];
  sprintf (tmp, "##%s@%ld##", __s, __unique);
  md5_crypt (tmp, SECURITY_MAGICK, dummy);
  strcpy (key, dummy + 8);
  strcpy (__s, key);
}

/**
 * Execute child process
 *
 * @param __cmd - command to execute
 */
static void
execute (char * __cmd)
{
  char **argv = 0;
  int argc = 0;

  cmd_parse_buf (__cmd, &argv, &argc);

  lrvm_ipc_send_command ("base", "");
  execv (argv[0], argv);

  cmd_free_arglist (argv, argc);
}

/**
 * Handler of TERM signals
 *
 * @param __signum - number of signal
 */
void
signal_term (int __signum)
{
  /* Caught one of terminating signals */
  /* Pipe is closen. Need to send this info to server */
  term ();
}

/**
 * Get user id by name
 *
 * @param __self - name of user to get ID of
 * @return user's ID
 */
static long
get_uid (const char *__self)
{
  struct passwd *p = getpwnam (__self);

  if (!p)
    {
      return -1;
    }

  return (long) p->pw_uid;
}

/**
 * Get group id by name
 *
 * @param __self - name of group to get ID of
 * @return group's ID
 */
static long
get_gid (char *__self)
{
  struct group *g = getgrnam (__self);

  if (!g)
    {
      return -1;
    }

  return (long) g->gr_gid;
}

/**
 * Prepare to change user and group
 *
 * @param __ruid - real UID
 * @param __rgid - real GID
 * @param __uid - effective UID
 * @param __gid - effective GID
 */
static void
chugid_prepare (long __ruid, long __rgid, long __uid, long __gid)
{
  char act[10], dst[10], name[1024];
  FILE *stream;
  int i, n, j;

  if (!__uid || !__gid || ((__uid < 0 || __gid < 0) && (!__ruid || !__rgid)))
    {
      lrvm_ipc_send_command ("exec_error", "\"Trying to execute process with "
                                           "ROOT provolegies\"");
      lrvm_ipc_done ();
      term ();
    }

  if (__uid == __ruid && __gid == __rgid)
    {
      return;
    }

  /* Open sequrity config */
  stream = fopen (config_file, "r");
  if (!stream)
    {
      lrvm_ipc_send_command ("exec_error",
                             "\"Cound not open configuration file\"");
      lrvm_ipc_done ();
      term ();
    }

  while (fscanf (stream, "%s %s %s", act, dst, name) > 0)
    {
      if (!strcmp (act, "allow"))
        {
          if (!strcmp (dst, "user"))
            {
              if (__uid == get_uid (name))
                {
                  allow_uid = 1;
                }
            }
          else
            if (!strcmp (dst, "group"))
            {
              if (__uid == get_gid (name))
                {
                  allow_gid = 1;
                }
            }
          else
            {
              if (!strcmp (dst, "usergroup"))
              {
                char user[128], group[128];
                i = j = 0;
                n = strlen (name);

                while (name[i] != ':' && i < n)
                  {
                    user[j++] = name[i++];
                  }
                user[j] = 0;

                i++;
                j = 0;
                while (i < n)
                  {
                    group[j++] = name[i++];
                  }
                group[j] = 0;

                if (__uid == get_uid (user) && __gid == get_gid (group))
                  {
                    allow_uid = allow_gid = 1;
                    break;
                  }
              }
            }
        }
    }

  fclose (stream);
}

/**
 * Change real and effective UID and GID
 *
 * @param __ruid - real UID
 * @param __rgid - real GID
 * @param __uid - effective UID
 * @param __gid - effective GID
 */
static void
chugid (long __ruid, long __rgid, long __uid, long __gid)
{
  if (__uid == __ruid && __gid == __rgid)
    {
      return;
    }

  if (setgroups (0, NULL) < 0)
    {
      lrvm_ipc_send_command ("exec_error", "\"Unable to call setgroups()\"");
      lrvm_ipc_done ();
      term ();
    }

  if (!allow_uid || !allow_gid)
    {
      lrvm_ipc_send_command ("exec_error","\"Execution with such GID and UID "
                                          "is not allowed\"");
      lrvm_ipc_done ();
      term ();
    }

  if (__gid >= 0 && __gid != __rgid && setregid (__gid, __gid) < 0)
    {
      lrvm_ipc_send_command ("exec_error",
                             "\"Unable to change effective GID\"");
      lrvm_ipc_done ();
      term ();
    }

  if (__uid >= 0 && __uid != __ruid && setreuid (__uid, __uid) < 0)
    {
      lrvm_ipc_send_command ("exec_error",
                             "\"Unable to change effective UID\"");
      lrvm_ipc_done ();
      term ();
    }
}

int
main (int __argc, char **__argv)
{
  pid_t pid, w;
  int i, status, config_allowed = 0;
  long port = -1;

  char cmd[4096] = "",
       workdir[4096] = "./",
       security[128],
       host[128],
       unique[16];

  char pchar_pid[8];
  char pchar_pid2[8];

  long uid = -1, gid = -1;
  long ruid = geteuid (), rgid = getegid ();
  int use_chroot = 0;

  /* Send PID of this stuff  */
  printf ("%u@", getpid ());
  fflush (0);

  signal (SIGINT, signal_term);
  signal (SIGHUP, signal_term);
  /*  signal (SIGSTOP, signal_term); */
  signal (SIGTERM, signal_term);

  strcpy (config_file, CONFIG_FILE);

  /* Getting command line parameters */
  for (i = 0; i < __argc; i++)
    {
      if (C_ARG_EQ ("-cmd") && i < __argc - 1)
        {
          strcpy (cmd, __argv[++i]);
        }
      if (C_ARG_EQ ("-workdir") && i < __argc - 1)
        {
          strcpy (workdir, __argv[++i]);
        }
      if (C_ARG_EQ ("-security") && i < __argc - 1)
        {
          strcpy (security, __argv[++i]);
        }
      if (C_ARG_EQ ("-port") && i < __argc - 1)
        {
          port = atoi (__argv[++i]);
        }
      if (C_ARG_EQ ("-host") && i < __argc - 1)
        {
          strcpy (host, __argv[++i]);
        }
      if (C_ARG_EQ ("-unique") && i < __argc - 1)
        {
          strcpy (unique, __argv[++i]);
        }
      if (C_ARG_EQ ("-sock-read-timeout") && i < __argc - 1)
        {
          read_timeout = atof (__argv[++i]) * NSEC_COUNT;
        }
      if (C_ARG_EQ ("-sock-read-delay") && i < __argc - 1)
        {
          read_delay = atof (__argv[++i]) * NSEC_COUNT;
        }

      if (C_ARG_EQ ("-uid") && i < __argc - 1)
        {
          uid = atol (__argv[++i]);
        }
      if (C_ARG_EQ ("-gid") && i < __argc - 1)
        {
          gid = atol (__argv[++i]);
        }

      if (C_ARG_EQ ("-config-file") && i < __argc - 1)
        {
          strcpy (config_file, __argv[++i]);
        }

      if (C_ARG_EQ ("-chroot"))
        {
          use_chroot = 1, i++;
        }
    }

  validate_security (security, atol (unique));

  /* Initialize IPC socket */
  lrvm_ipc_init (host, port);

  /****
   * Send basic info to LibRUN
   */

  sprintf (pchar_pid, "%u", getpid ());

  lrvm_ipc_send_command ("unique", unique);
  lrvm_ipc_send_command ("security", security);
  lrvm_ipc_send_command ("validate", "");

  /* Check for valid prefix of config file */
  i = 0;
  while (config_allowed_prefixes[i])
    {
      if (!strncmp (config_file, config_allowed_prefixes[i],
                    strlen (config_allowed_prefixes[i])))
        {
          config_allowed = 1;
          break;
        }
      i++;
    }
  if (!config_allowed)
    {
      /* lrvm_ipc_send_command ("exec_error", config_file); */
      lrvm_ipc_send_command ("exec_error", "\"Strange config file. "
                             "LRVM not started because of security reasons.\"");
      lrvm_ipc_done ();
      term ();
    }

  switch (pid = fork ()) /* Deattach from parent */
    {
    case -1 :
      term ();
      return -1;

    case 0:
      chugid_prepare (ruid, rgid, uid, gid);

      chdir (workdir);
      if (use_chroot)
        {
          chroot (workdir);
        }

      chugid (ruid, rgid, uid, gid);

      sprintf (pchar_pid2, "%u", getpid ());
      lrvm_ipc_send_command ("taskpid", pchar_pid2);

      execute (cmd);
      exit (errno);
    }

  do
    {
      w = waitpid (-1, &status, WUNTRACED | WCONTINUED);
      if (w == -1)
        {
          return -1;
        }
      if (WIFEXITED (status))
        {
          int exit_code = WEXITSTATUS (status);
          char buf[8];
          sprintf (buf, "%d", exit_code);
          lrvm_ipc_send_command ("exit_code", buf);
        }
      else
        if (WIFSIGNALED (status))
        {
          int termsig = WTERMSIG (status);
          char buf[8];
          sprintf (buf, "%d", termsig);
          lrvm_ipc_send_command ("termsig", buf);
        }
      else
        if (WIFSTOPPED (status))
        {
          int stopsig = WSTOPSIG (status);
          char buf[8];
          sprintf (buf, "%d", stopsig);
          lrvm_ipc_send_command ("stopsig", buf);
        }
      else
        if (WIFCONTINUED (status))
        {
          lrvm_ipc_send_command ("continue", "");
        }
    }
  while (!WIFEXITED (status) && !WIFSIGNALED (status));

  fflush (0);
  lrvm_ipc_send_command ("finalize", "");
  lrvm_ipc_done ();
  exit (0);
}
