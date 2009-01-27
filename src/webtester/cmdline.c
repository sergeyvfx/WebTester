/**
 * WebTester Server - server of on-line testing system
 *
 * Command line parameters' parser
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */


#include "cmdline.h"

#include <libwebtester/core.h>

#include <string.h>
#include <stdlib.h>

#include "autoinc.h"
#include "core.h"

#define C_ARG_EQ(__s)  (!strcmp (__argv[i], __s))
#define CHECK_NEXT     (i<__argc-1)
#define NEXT_ARG       (__argv[++i])

static char usage[] = "Usage: webtester [options]\n"
        "  -s,  --silent         use specified config file\n"
        "  -c,  --config-file    Turn to silent mode\n"
#ifdef USER_DEBUG
        "  -d,  --debug          Turn CORE to debug mode\n"
#endif
        "  -h,  --help           Print this message and quit\n"
        "  -v,  --version        Print version of CORE and quit\n";

void
wt_cmdline_parse_args (int __argc, char **__argv)
{
  int i;

  for (i = 1; i < __argc; i++)
    {
      /* Silent output */
      if (C_ARG_EQ ("--silent") || C_ARG_EQ ("-s"))
        {
          core_set_silent (TRUE);
        }
      else if (C_ARG_EQ ("--config-file") || C_ARG_EQ ("-c"))
        {
          if (CHECK_NEXT)
            {
              wt_set_config_file (NEXT_ARG);
            }
        }
      else if (C_ARG_EQ ("--version") || C_ARG_EQ ("-v"))
        {
          printf ("%s\n", core_get_version_string ());
          exit (0);
        }
      else if (C_ARG_EQ ("--help") || C_ARG_EQ ("-h"))
        {
          printf ("%s\n\n", core_get_version_string ());
          printf (usage);
          exit (0);
        }
      else
#ifdef USER_DEBUG
        if (C_ARG_EQ ("--debug") || C_ARG_EQ ("-d"))
        {
          core_enter_debug_mode ();
        }
      else
#endif
        {
          printf ("Option unknown: %s\n\n", __argv[i]);
          printf (usage);
          exit (-1);
        }
    }
}
