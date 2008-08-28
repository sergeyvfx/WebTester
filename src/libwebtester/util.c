/*
 *
 * ================================================================================
 *  util.h
 * ================================================================================
 *
 *  Some useful procedures
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "util.h"
#include "smartinclude.h"
#include "strlib.h"
#include "cmd.h"

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <memory.h>
#include <unistd.h>

#include <glib.h>

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

static int
is_number_entry                    (char *__self, int __float)
{
  int i, n=strlen (__self);
  int decimal=0;
  for (i=0; i<n; i++)
    {
      if (__self[i]=='-' && i!=0) return 0;
      if (__self[i]=='.' || __self[i]==',')
        {
          if (!__float) return 0;
          if (decimal) return 0;
          decimal=1;
          continue;
        }
      if ((__self[i]<'0' || __self[i]>'9') && __self[i]!='-') return 0;
    }
  return 1;
}

int
is_number                          (char *__self)
{
  return is_number_entry (__self, 1);
}

int
is_integer                         (char *__self)
{
  return is_number_entry (__self, 0);
}

void
drop_triling_zeroes                (char *__self)
{
  int uk=strlen (__self)-1;
  while (__self[uk]=='0')
    __self[uk]=0,
    uk--;
  if (__self[uk]=='.' || __self[uk]==',')
    __self[uk]=0;
}

char*
sys_parse                          (char *__data, char *__token)
{
  int len=0;
  char ch;
  __token[len]=0;
  if (!__data || !*__data) return 0;
//__skip_while:
  ch=*__data;
  while ((ch<=32 || ch>=127) && ch)
    ch=*(++__data);
    
  if (ch=='"')
    {
      ch=*(++__data);
      while (ch!='"' && ch)
        {
          if (ch=='\\')
            {
              ch=*(++__data);
              if (ch=='n') *(__token+len++)='\n'; else
              if (ch=='r') *(__token+len++)='\r'; else
              if (ch=='t') *(__token+len++)='\t'; else
                *(__token+len++)=ch;
            } else *(__token+len++)=ch;
          ch=*(++__data);
        }
      __data++;
    } else
    while (ch>32 && ch<127)
      {
        *(__token+len++)=ch;
        ch=*(++__data);
      }

  *(__token+len)=0;
  return __data;
}

int
fname                              (char *__full, char *__out)
{
  int i, n=strlen (__full),lastuk=0, len=0;
  for (i=0; i<n; i++) if (__full[i]=='/') lastuk=i;
  for (i=lastuk+1; i<n; i++) __out[len++]=__full[i];
  __out[len]=0;
  return 0;
}

int
dirname                            (char *__full, char *__out)
{
  int i, n=strlen (__full),lastuk=0, len=0;
  for (i=0; i<n; i++) if (__full[i]=='/') lastuk=i;
  for (i=0; i<lastuk; i++) __out[len++]=__full[i];
  __out[len]=0;
  return 0;
}

int
dropextension                      (char *__fn, char *__out)
{
  int firstuk=0,i,n,len=0;
  n=strlen (__fn);
  for (i=0; i<n; i++) if (__fn[i]=='.') {firstuk=i; break;}
  for (i=0; i<firstuk; i++) __out[len++]=__fn[i];
  __out[len]=0;
  return 0;
}

int
getextension                       (char *__fn, char *__out)
{
  int lastuk=0,i,n,len=0;
  n=strlen (__fn);
  for (i=0; i<n; i++) if (__fn[i]=='.') lastuk=i;
  for (i=lastuk+1; i<n; i++) __out[len++]=__fn[i];
  __out[len]=0;
  return 0;
}

int
sys_launch                         (char *__args, ...)
{
  char buf[65536], dummy[65536];
  int ret;

  PACK_ARGS (__args, dummy, 1024);
  sprintf (buf, "%s > /dev/null 2>&1", dummy);

  ret=system (buf);
  if (WIFSIGNALED (ret) && (WTERMSIG (ret)==SIGINT || WTERMSIG (ret)==SIGQUIT))
    core_kill_process (0, WTERMSIG (ret));

  return 0;
}

timeval_t
now                                (void)
{
  timeval_t res;
  gettimeofday (&res, 0);
  return res;
}

timeval_t
timedist                           (timeval_t __from, timeval_t __to)
{
  timeval_t res;

  res.tv_sec=0;
  res.tv_usec=0;

  if ( (__from.tv_sec>__to.tv_sec) ||
       (__from.tv_sec==__to.tv_sec && __from.tv_usec>__to.tv_usec)
     )
    return res;

  res.tv_sec=__to.tv_sec-__from.tv_sec;

  if (__to.tv_usec>=__from.tv_usec)
    {
      res.tv_usec=__to.tv_usec-__from.tv_usec;
    } else
    {
      res.tv_sec--;
      res.tv_usec=__to.tv_usec+1000000-__from.tv_usec;
    }

  return res;
}

int
tv_usec_cmp                        (timeval_t __tv, __u64_t __usec)
{
  __u64_t sec, usec;
  sec=__usec/1000000;
  usec=__usec%1000000;

  if (__tv.tv_sec>sec) return 1;
  if (__tv.tv_sec<sec) return -1;
  if (__tv.tv_usec>usec) return 1;
  if (__tv.tv_usec<usec) return -1;
  return 0;
}

timeval_t
timedistnow                        (timeval_t __from)
{
  timeval_t to=now ();
  return timedist (__from, to);
}

void
prepare_cmdarg                     (char *__src, char *__dst)
{
  int i, n=strlen (__src), clen=0;
  char *buf=malloc (n*2+1);
  char ch;

  for (i=0; i<n; i++)
    {
      ch=__src[i];
      if (ch==' ' || ch=='!' || ch=='&' || ch=='|' || ch==';' || ch=='>' ||
          ch=='(' || ch==')')
        buf[clen++]='\\';
      buf[clen++]=ch;
    }

  buf[clen]=0;
  strcpy (__dst, buf);
  free (buf);
}

long
uid_by_name                        (char *__name)
{
  struct passwd *p=getpwnam (__name);
  if (!p) return -1;
  return (long)p->pw_uid;
}

long
gid_by_name                        (char *__name)
{
  struct group *g=getgrnam (__name);
  if (!g) return -1;
  return (long)g->gr_gid;
}

void
get_datetime_strf                  (char *__out, int __size, char *__format)
{
  time_t t;
  struct tm *tm;
  t=time (0);
  tm=localtime (&t);
  strftime (__out, __size, __format, tm);
}

int
is_truth                          (char *__self)
{
  int res=0;
  char *pchar;
  if (is_number (__self))
    return atol (__self)!=0;
  pchar=malloc (strlen (__self)+1);
  strupr (__self, pchar);
  if (!strcmp (pchar, "TRUE") || !strcmp (pchar, "T"))
    res=1;
  free (pchar);
  return res;
}

double
fabs                               (double __self)
{
  return ((__self>0)?(__self):(-__self));
}

double
sign                               (double __self)
{
  if (fabs (__self)<1e-8) return 0;
  if (__self>0) return 1;
  return -1;
}
