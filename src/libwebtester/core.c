/*
 *
 * =============================================================================
 *  core.h
 * =============================================================================
 *
 *  Some core built-in stuff
 *
 *  Written (by Nazgul) under GPL
 *
*/

#include "core.h"
#include "smartinclude.h"

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
#  else
  static char special[] = "Stable";
#  endif
#endif

static struct utsname utsName;
static char core_version_string[65536];
static time_t  start_time=0;

int
core_init                          (void)
{
  start_time=time (0);
  core_init_version_string ();

  return 0;
}

void
core_done                          (void)
{
  core_unregister_paths ();
}

time_t
core_get_starttime                 (void)
{
  return start_time;
}

time_t
core_get_uptime                    (void)
{
  return time (0)-start_time;
}

void
core_init_version_string           (void)
{
  uname (&utsName);
  // Version string
  sprintf (core_version_string, "%s v%s [%s][`%s` `%s %s` on %s][%s]",
    CORE_PACKAGE_NAME,
    CORE_VERSION,
    buildType,
    OSKind,
    buildDate,
    buildTime,
    BUILD_HOST,
    special);
}

char*
core_get_version_string            (void)
{
  return core_version_string;
}


void
core_oops                          (char *__text, ...)
{
  char print_buf[65536];
  PACK_ARGS (__text, print_buf, 65536);
  printf ("Core oops: %s\n", __text);
  exit (-1);
}
