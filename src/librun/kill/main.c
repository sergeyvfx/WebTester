/*
 * ================================================================================
 *  kill.c - part of the lrvm - client part of the killer of LibRUN
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
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

#define CONFIG_FILE            "/etc/lrvm.conf"

#define C_ARG_EQ(__s) (!strcmp (__argv[i], __s))

static char *config_allowed_prefixes[]={
  "/etc",
  "/opt/webtester",
  "/home/webtester",
  0
};

static char config_file[4096];

static long
get_uid                            (char *__self)
{
  struct passwd *p=getpwnam (__self);
  if (!p) return -1;
  return (long)p->pw_uid;
}

static long
get_gid                            (char *__self)
{
  struct group *g=getgrnam (__self);
  if (!g) return -1;
  return (long)g->gr_gid;
}

void
chugid                             (long __ruid, long __rgid, long __uid, long __gid)
{
  char act[10], dst[10], name[1024];
  FILE *stream;
  int allow_uid=0, allow_gid=0, i, n, j;

  if (!__uid || !__gid || ((__uid<0 || __gid<0) && (!__ruid || !__rgid)))
    exit (1);

  if (__uid==__ruid && __gid==__rgid)
    return;

  // Open sequrity config
  stream=fopen (config_file, "r");
  if (!stream)
    exit (1);

  while (fscanf (stream, "%s %s %s", act, dst, name)>0)
    {
      if (!strcmp (act, "allow"))
        {
          if (!strcmp (dst, "user"))
            {
              if (__uid==get_uid (name)) allow_uid=1;
            } else
          if (!strcmp (dst, "group"))
            {
              if (__uid==get_gid (name)) allow_gid=1;
            } else
          if (!strcmp (dst, "usergroup"))
            {
              char user[128], group[128];
              i=j=0; n=strlen (name);

              while (name[i]!=':' && i<n)
                user[j++]=name[i++];
              user[j]=0;
              
              i++; j=0;
              while (i<n)
                group[j++]=name[i++];
              group[j]=0;
              
              if (__uid==get_uid (user) && __gid==get_gid (group))
                {
                  allow_uid=allow_gid=1;
                  break;
                }
            }
        }
    }

  fclose (stream);

  if (setgroups (0, NULL)<0)
    exit (1);
  
  if (!allow_uid || !allow_gid)
    exit (1);

  if (__gid>=0 && __gid!=__rgid && setregid (__gid, __gid)<0)
    exit (1);

  if (__uid>=0 && __uid!=__ruid && setreuid (__uid, __uid)<0)
    exit (1);
}

int
main                               (int __argc, char **__argv)
{
  int i, config_allowed=0;

  long uid=-1, gid=-1;
  long ruid=geteuid (), rgid=getegid ();
  long pid=0, sig=0;

  strcpy (config_file, CONFIG_FILE);

  // Getting command line parameters
  for (i=0; i<__argc; i++)
    {
      if (C_ARG_EQ ("-uid") && i<__argc-1)
        uid=atol (__argv[++i]);
      if (C_ARG_EQ ("-gid") && i<__argc-1)
        gid=atol (__argv[++i]);

      if (C_ARG_EQ ("-pid") && i<__argc-1)
        pid=atol (__argv[++i]);
      if (C_ARG_EQ ("-signal") && i<__argc-1)
        sig=atol (__argv[++i]);

      if (C_ARG_EQ ("-config-file") && i<__argc-1)
        strcpy (config_file, __argv[++i]);
    }

  i=0;
  while (config_allowed_prefixes[i])
    {
      if (!strncmp (config_file, config_allowed_prefixes[i], strlen (config_allowed_prefixes[i])))
        {
          config_allowed=1;
          break;
        }
      i++;
    }

  if (!config_allowed)
    exit (1);

  chugid (ruid, rgid, uid, gid);

  if (pid<=0)
    exit (-1);

  kill (pid, sig);

  exit (0);

  return 0;
}
