/**
 * WebTester Server - server of on-line testing system
 *
 * Some core built-in stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "core.h"

#include <stdio.h>
#include <sys/utsname.h>
#include <stdarg.h>
#include <stdlib.h>

static char buildDate[] = __DATE__;
static char buildTime[] = __TIME__;

#ifdef __DEBUG
  static char buildType[] = "Debug";
#else
  static char buildType[] = "Release";
#endif

#ifdef LINUX
  static char OSKind[] = "Linux";
#endif

#ifdef __UNSTABLE
  static char special[] = "Unstable";
#else
#  ifdef __TESTING
  static char special[] = "Testing";
# else
  static char special[] = "Stable";
#  endif
#endif

static struct utsname utsName;
static char core_version_string[65536];
static time_t start_time = 0;

/**
 * Initialize CORE
 *
 * @return zero on success, non-zero otherwise
 */
int
core_init (void)
{
  start_time = time (0);
  core_init_version_string ();

  return 0;
}

/**
 * Uninitialize CORE
 */
void
core_done (void)
{
  core_unregister_paths ();
  core_io_done ();
}

/**
 * Get time when CORE was started
 *
 * @return time when CORE was started
 */
time_t
core_get_starttime (void)
{
  return start_time;
}

/**
 * Get uptime of CORE
 *
 * @return uptime of CORE
 */
time_t
core_get_uptime (void)
{
  return time (0) - start_time;
}

/**
 * Initialize version string
 */
void
core_init_version_string (void)
{
  uname (&utsName);
  // Version string
  snprintf (core_version_string, BUF_SIZE (core_version_string),
            "%s v%s [%s][`%s` `%s %s` on %s][%s]",
            CORE_PACKAGE_NAME,
            CORE_VERSION,
            buildType,
            OSKind,
            buildDate,
            buildTime,
            BUILD_HOST,
            special);
}

/**
 * Get version string
 *
 * @return version string
 */
char*
core_get_version_string (void)
{
  return core_version_string;
}

/**
 * CORE has been crashed :(
 *
 * @param __text - message to print
 */
void
core_oops (char *__text, ...)
{
  char print_buf[65536];
  PACK_ARGS (__text, print_buf, 65536);
  printf ("Core oops: %s\n", __text);
  exit (-1);
}
