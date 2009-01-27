/**
 * WebTester Server - server of on-line testing system
 *
 * IPC blacklisting stuff module
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "ipc.h"
#include "util.h"
#include "dynastruc.h"
#include "package.h"

#include <string.h>
#include <malloc.h>
#include <memory.h>

static BOOL active = FALSE;
static char *blacklist_file;

static dynastruc_t *blacklist;
static long reset_timeout;

static char banner[] = "# This file is automatically saved by " PACKAGE;

#define GOOD_OCTET(__a) \
  (a>=0 && a<=255)

typedef struct
{
  char ip[32];
  time_t timestamp;
} blacklist_item_t;

/**
 * Blacklist IP address
 *
 * @param __ip - ip address to blacklist
 * @param __time - blacklisted time
 */
static void
blacklist_ip (const char *__ip, time_t __time)
{
  blacklist_item_t *ptr;

  if (__time == (time_t) (-1))
    {
      __time = time (0);
    }

  ptr = malloc (sizeof (blacklist_item_t));
  strcpy (ptr->ip, __ip);
  ptr->timestamp = __time;

  dyna_append (blacklist, ptr, 0);
}

/**
 * Comparator to find blacklist information by IP address
 *
 * @param __l - currently overviewing blacklist descriptor
 * @param __r - needed IP
 * @return non-zero on match, zero otherwise
 */
static int
blacklist_ip_comparator (void *__l, void *__r)
{
  blacklist_item_t *item = __l;
  char *ip = __r;

  return !strcmp (item->ip, ip);
}

/**
 * Remove IP address from blacklist
 *
 * @param __ip - ip address to unblacklist
 */
static void
unblacklist_ip (const char *__ip)
{
  dyna_item_t *item;

  do
    {
      item = dyna_search (blacklist, (void*)__ip, 0, blacklist_ip_comparator);
      dyna_delete (blacklist, item, dyna_deleter_free_ref_data);
    }
  while (item);
}

/**
 * Parser iterator for file parser
 *
 * @param __self - string to parse
 */
static void
parse_blacklist_entry (char *__self)
{
  int a, b, c, d, e, f;
  int ipv6 = 0;
  long ltime;
  struct tm tm;
  time_t t;

  while ((*__self) <= ' ' && *__self)
    {
      __self++;
    }

  /* Comment */
  if (*__self == '#' || *__self == ';')
    {
      return;
    }

  /* Try to parse as IPv4 */
  if (sscanf (__self, "%d.%d.%d.%d %ld-%d:%d:%d\n", &a, &b, &c, &d,
              &ltime, &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 8)
    {
      /* Try to parse as IPv6 */
      if (sscanf (__self, "%d.%d.%d.%d.%d.%d %ld-%d:%d:%d\n", &a, &b, &c, &d,
                  &e, &f, &ltime, &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 10)
        {
          return;
        }
      ipv6 = 1;
    }

  /* Check IP is valid */
  if (!GOOD_OCTET (a) || !GOOD_OCTET (b) || !GOOD_OCTET (c) ||
      !GOOD_OCTET (d) || (ipv6 && (!GOOD_OCTET (e) || !GOOD_OCTET (f))))
    {
      return;
    }

  /* Unpack date */
  tm.tm_year = ltime / 10000 - 1900;
  tm.tm_mon = ltime / 100 % 100 - 1;
  tm.tm_mday = ltime % 100 - 1;

  /* Build unixtime */
  t = mktime (&tm);

  if (t != (time_t) (-1))
    {
      char ip[32];

      if (!ipv6)
        {
          sprintf (ip, "%d.%d.%d.%d", a, b, c, d);
        }
      else
        {
          sprintf (ip, "%d.%d.%d.%d.%d.%d", a, b, c, d, e, f);
        }

      blacklist_ip (ip, t);
    }
}

/**
 * Read blacklisted IPs from file
 */
static void
read_blacklisted (void)
{
  char token[1024] = {0};

  FILE *stream = fopen (blacklist_file, "r");

  if (!stream)
    {
      return;
    }

  fgets (token, 1024, stream);
  while (!feof (stream))
    {
      parse_blacklist_entry (token);
      memset (token, 0, sizeof (token));
      fgets (token, 1024, stream);
    }

  if (token[0])
    {
      parse_blacklist_entry (token);
    }

  fclose (stream);
}

/**
 * Flush blacklist to file
 */
static void
flush_blacklist (void)
{
  FILE *stream = fopen (blacklist_file, "w");

  if (!stream)
    {
      return;
    }

  blacklist_item_t *cur;
  struct tm *data;

  fprintf (stream, "%s\n\n", banner);

  DYNA_FOREACH (blacklist, cur)
    data = localtime (&cur->timestamp);
    fprintf (stream, "%s\t\t%d%02d%02d-%02d:%02d:%02d\n", cur->ip,
             data->tm_year + 1900, data->tm_mon + 1, data->tm_mday + 1,
             data->tm_hour, data->tm_min, data->tm_sec);
  DYNA_DONE

  fclose (stream);
}

/****
 * User's backend
 */

/**
 * Initialzie IPC blacklist stuff
 *
 * @param __blacklist_file - file with list of blacklisted IPs
 * @param __reset_timeout - unblacklist interval
 * @return zero on success, non-zero otherwise
 */
int
ipc_blacklist_init (const char *__blacklist_file, long __reset_timeout)
{
  if (!__blacklist_file)
    {
      return -1;
    }

  active = TRUE;
  blacklist_file = strdup (__blacklist_file);
  reset_timeout = __reset_timeout;

  blacklist = dyna_create ();

  read_blacklisted ();

  return 0;
}

/**
 * Uninitialize IPC blacklist stuff
 */
void
ipc_blacklist_done (void)
{
  if (!active)
    {
      return;
    }

  active = FALSE;
  free (blacklist_file);
  dyna_destroy (blacklist, dyna_deleter_free_ref_data);
}

/**
 * Check is IP address blacklisted
 *
 * @param __ip - IP address to check
 * @return zero if IP address is not blacklisted, non-zero otherwise
 */
int
ipc_blacklisted (const char *__ip)
{
  time_t t = time (0);
  blacklist_item_t *cur;
  BOOL res = FALSE, changed = FALSE;

  if (!active)
    {
      return FALSE;
    }

  DYNA_FOREACH (blacklist, cur)
    if (reset_timeout > 0 && t - cur->timestamp >= reset_timeout)
      {
        DYNA_DELETE_CUR (dyna_deleter_free_ref_data);
        changed = TRUE;
      }
    else
      {
        if (!strcmp (__ip, cur->ip))
          {
            res = TRUE;
          }
      }
  DYNA_DONE

  if (changed)
    {
      flush_blacklist ();
    }

  return res;
}

/**
 * Blacklist IP
 *
 * @param __ip - ip address to blacklist
 * @param __time - blacklisted time
 */
void
ipc_blacklist_ip (const char *__ip, time_t __time)
{
  blacklist_ip (__ip, __time);
  flush_blacklist ();
  core_print (MSG_INFO, "IPC: IP %s blacklisted\n", __ip);
}

/**
 * Remove IP address from blacklist
 *
 * @param __ip - ip address to unblacklist
 */void
ipc_unblacklist_ip (const char *__ip)
{
  unblacklist_ip (__ip);
  core_print (MSG_INFO, "IPC: IP %s is now not in blacklist\n", __ip);
}
