/**
 * WebTester Server - server of on-line testing system
 *
 * Some usefull procedures
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
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

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

/**
 * Common part of in_number and is_integer
 *
 * @param __self - number in string
 * @param __float - is input number is floating-point
 * @return non-zero if given string is a number, zero otherwise
 */
static int
is_number_entry (const char *__self, int __float)
{
  int i, n = strlen (__self);
  int decimal = 0;
  for (i = 0; i < n; i++)
    {
      if (__self[i] == '-' && i != 0)
        {
          return 0;
        }
      if (__self[i] == '.' || __self[i] == ',')
        {
          if (!__float)
            {
              return 0;
            }
          if (decimal)
            {
              return 0;
            }
          decimal = 1;
          continue;
        }
      if ((__self[i] < '0' || __self[i] > '9') && __self[i] != '-')
        {
          return 0;
        }
    }
  return 1;
}

/**
 * Check is string a number
 *
 * @param __self - number in string
 * @return non-zero if given string is a number, zero otherwise
 */
int
is_number (const char *__self)
{
  return is_number_entry (__self, 1);
}

/**
 * Check is string an integer number
 *
 * @param __self - number in string
 * @return non-zero if given string is a number, zero otherwise
 */
int
is_integer (const char *__self)
{
  return is_number_entry (__self, 0);
}

/**
 * Drop trilling zeroes from number in string
 *
 * @param __self - string to operate with
 */
void
drop_triling_zeroes (char *__self)
{
  int uk = strlen (__self) - 1;
  while (__self[uk] == '0')
    {
      __self[uk] = 0;
      uk--;
    }
  if (__self[uk] == '.' || __self[uk] == ',')
    {
      __self[uk] = 0;
    }
}

/**
 * System parser
 *
 * @param __data - input string
 * @param __token - catched token
 * @return new shift of data string
 */
char*
sys_parse (char *__data, char *__token)
{
  int len = 0;
  char ch;
  __token[len] = 0;
  if (!__data || !*__data)
    {
      return 0;
    }
  ch = *__data;
  while ((ch <= 32 || ch >= 127) && ch)
    {
      ch = *(++__data);
    }

  if (ch == '"')
    {
      ch = *(++__data);
      while (ch != '"' && ch)
        {
          if (ch == '\\')
            {
              ch = *(++__data);
              if (ch == 'n') *(__token + len++) = '\n'; else
              if (ch == 'r') *(__token + len++) = '\r'; else
              if (ch == 't') *(__token + len++) = '\t'; else
                *(__token + len++) = ch;
            }
          else
            {
              *(__token + len++) = ch;
            }
          ch = *(++__data);
        }
      __data++;
    }
  else
    while (ch > 32 && ch < 127)
      {
        *(__token + len++) = ch;
        ch = *(++__data);
      }

  *(__token + len) = 0;
  return __data;
}

/**
 * Get name of file
 *
 * @param __full - full name of file
 * @param __out - name of file
 * @return zero on success, non-zero otherwise
 */
int
fname (const char *__full, char *__out)
{
  int i, n = strlen (__full), lastuk = 0, len = 0;
  for (i = 0; i < n; i++) if (__full[i] == '/')
    {
      lastuk = i;
    }
  for (i = lastuk + 1; i < n; i++)
    {
      __out[len++] = __full[i];
    }
  __out[len] = 0;
  return 0;
}

/**
 * Get name of a directory
 *
 * @param __full - full name of file
 * @param __out - directory name
 * @return zero on success, non-zero otherwise
 */
int
dirname (const char *__full, char *__out)
{
  int i, n = strlen (__full), lastuk = 0, len = 0;
  for (i = 0; i < n; i++) if (__full[i] == '/')
    {
      lastuk = i;
    }
  for (i = 0; i < lastuk; i++)
    {
      __out[len++] = __full[i];
    }
  __out[len] = 0;
  return 0;
}

/**
 * Drop extension from name of file
 *
 * @param __fn - name of file
 * @param __out - name of file without extension
 * @return zero on success, non-zero otherwise
 */
int
dropextension (const char *__fn, char *__out)
{
  int firstuk = 0, i, n, len = 0;
  n = strlen (__fn);
  for (i = 0; i < n; i++)
    {
      if (__fn[i] == '.')
        {
          firstuk = i;
          break;
        }
    }
  for (i = 0; i < firstuk; i++)
    {
      __out[len++] = __fn[i];
    }
  __out[len] = 0;
  return 0;
}

/**
 * Get file's extension
 *
 * @param __fn - name of file
 * @param __out - file's extension
 * @return zero on success, non-zero otherwise
 */
int
getextension (const char *__fn, char *__out)
{
  int lastuk = 0, i, n, len = 0;
  n = strlen (__fn);
  for (i = 0; i < n; i++)
    {
      if (__fn[i] == '.')
        {
          lastuk = i;
        }
    }
  for (i = lastuk + 1; i < n; i++)
    {
      __out[len++] = __fn[i];
    }
  __out[len] = 0;
  return 0;
}

/**
 * Launch command
 *
 * @params __args - arguments
 * @return zero on success, non-zero otherwise
 */
int
sys_launch (const char *__args, ...)
{
  char buf[65536], dummy[65536];
  int ret;

  PACK_ARGS (__args, dummy, 65535);
  snprintf (buf, BUF_SIZE (buf), "%s > /dev/null 2>&1", dummy);

  ret = system (buf);
  if (WIFSIGNALED (ret) && (WTERMSIG (ret) == SIGINT ||
      WTERMSIG (ret) == SIGQUIT))
    {
      core_kill_process (0, WTERMSIG (ret));
    }

  return 0;
}

/**
 * Get current time
 *
 * @return current time
 */
timeval_t
now (void)
{
  timeval_t res;
  gettimeofday (&res, 0);
  return res;
}

/**
 * Get distance between two times
 *
 * @param __from - start timestamp
 * @param __to - finito timestamp
 * @return distance between given timestamps
 */
timeval_t
timedist (timeval_t __from, timeval_t __to)
{
  timeval_t res;

  res.tv_sec = 0;
  res.tv_usec = 0;

  if ((__from.tv_sec > __to.tv_sec) ||
      (__from.tv_sec == __to.tv_sec && __from.tv_usec > __to.tv_usec)
      )
    {
      return res;
    }

  res.tv_sec = __to.tv_sec - __from.tv_sec;

  if (__to.tv_usec >= __from.tv_usec)
    {
      res.tv_usec = __to.tv_usec - __from.tv_usec;
    }
  else
    {
      res.tv_sec--;
      res.tv_usec = __to.tv_usec + 1000000 - __from.tv_usec;
    }

  return res;
}

/**
 * Compare timeval and milliseconds
 *
 * @param __tv - timeval
 * @param __usec - milliseconds
 * @return the same as strcmp()
 */
int
tv_usec_cmp (timeval_t __tv, __u64_t __usec)
{
  __u64_t sec, usec;
  sec = __usec / 1000000;
  usec = __usec % 1000000;

  if (__tv.tv_sec > sec)
    {
      return 1;
    }
  if (__tv.tv_sec < sec)
    {
      return -1;
    }
  if (__tv.tv_usec > usec)
    {
      return 1;
    }
  if (__tv.tv_usec < usec)
    {
      return -1;
    }
  return 0;
}

/**
 * Get distance from given timestamp and current timestamp
 *
 * @param __from - base timestamp
 * @return distance from given and current timestamps
 */
timeval_t
timedistnow (timeval_t __from)
{
  timeval_t to = now ();
  return timedist (__from, to);
}

/**
 * Prepare argument for command line
 *
 * @param __src - source string
 * @param __dst - destination string
 */
void
prepare_cmdarg (const char *__src, char *__dst)
{
  int i, n = strlen (__src), clen = 0;
  char *buf = malloc (n * 2 + 1);
  char ch;

  for (i = 0; i < n; i++)
    {
      ch = __src[i];
      if (ch == ' ' || ch == '!' || ch == '&' || ch == '|' || ch == ';'||
          ch == '>' || ch == '(' || ch == ')')
        {
          buf[clen++] = '\\';
        }
      buf[clen++] = ch;
    }

  buf[clen] = 0;
  strcpy (__dst, buf);
  free (buf);
}

/**
 * Get user's id by its name
 *
 * @param __name - name of user
 * @return id of user
 */
long
uid_by_name (const char *__name)
{
  struct passwd *p = getpwnam (__name);
  if (!p)
    {
      return -1;
    }
  return (long) p->pw_uid;
}

/**
 * Get group's id by its name
 *
 * @param __name - name of group
 * @return id of group
 */
long
gid_by_name (const char *__name)
{
  struct group *g = getgrnam (__name);
  if (!g)
    {
      return -1;
    }
  return (long) g->gr_gid;
}

/**
 * Format current date with given format string
 *
 * @param __out - result will be stored here
 * @param __size - size of buffer
 * @param __format - format string
 */
void
get_datetime_strf (char *__out, int __size, const char *__format)
{
  time_t t;
  struct tm *tm;
  t = time (0);
  tm = localtime (&t);
  strftime (__out, __size, __format, tm);
}

/**
 * Check is given string is a truth value
 *
 * @param __self - string
 * @return non-zero if string is truth, zero otherwise
 */
int
is_truth (const char *__self)
{
  int res = 0;
  char *pchar;
  if (is_number (__self))
    {
      return atol (__self) != 0;
    }
  pchar = malloc (strlen (__self) + 1);
  strupr (__self, pchar);
  if (!strcmp (pchar, "TRUE") || !strcmp (pchar, "T"))
    {
      res = 1;
    }
  free (pchar);
  return res;
}

/**
 * Floating absolutely value
 *
 * @param __self - input number
 * @return absolyte value
 */
double
fabs (double __self)
{
  return ((__self > 0) ? (__self) : (-__self));
}

/**
 * Get sign of number
 *
 * @param __self - input number
 * @reutrn sign of number
 */
double
sign (double __self)
{
  if (fabs (__self) < 1e-8)
    {
      return 0;
    }
  if (__self > 0)
    {
      return 1;
    }
  return -1;
}

/**
 * Parse range string
 *
 * @param __range - range to parse
 * @param __from - LO boundary of range
 * @param __to - HI boundary of range
 * @return zero on success, non-zero otherwise
 */
int
parse_range (const char *__range, long *__from, long *__to)
{
  int i, n, len = 0, state = 0;
  char token[16];
  char ch;

  i = 0;
  n = strlen (__range);

  while (i < n)
    {
      ch = __range[i];

      if (ch >= '0' && ch <= '9')
        {
          if (len > 15)
            {
              /* Token is too long */
              return -1;
            }

          token[len++] = ch;
        }
      else if (ch == '-')
        {
          if (len == 0)
            {
              /* No LO boundary */
              return -1;
            }

          if (state == 1)
            {
              /* Duplicate dashes in range */
              return -1;
            }

          token[len] = 0;
          *__from = atol (token);
          state = 1;
          len = 0;
        }
      else
        {
          /* Invalid character */
          return -1;
        }

      ++i;
    }

  if (len > 0)
    {
      token[len] = 0;
      *__to = atol (token);
      return 0;
    }

  return -1;
}
