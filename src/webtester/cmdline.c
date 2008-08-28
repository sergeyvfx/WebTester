/*
 *
 * ================================================================================
 *  cmdline.c - part of the WebTester Server
 * ================================================================================
 *
 *  Command line parameter parser
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <libwebtester/smartinclude.h>

#include <string.h>
#include "cmdline.h"
#include "autoinc.h"
#include "core.h"

#define C_ARG_EQ(__s)  (!strcmp (__argv[i], __s))
#define CHECK_NEXT     (i<__argc-1)
#define NEXT_ARG       (__argv[++i])

void
wt_cmdline_parse_args              (int __argc, char **__argv)
{
  int i;
  for (i=0; i<__argc; i++)
    {
      // Silent output
      if (C_ARG_EQ ("--silent") || C_ARG_EQ ("-s"))
        {
          core_set_silent (TRUE);
        }
      if (C_ARG_EQ ("--config-file") || C_ARG_EQ ("-c"))
        {
          if (CHECK_NEXT)
            wt_set_config_file (NEXT_ARG);
        }
    }
}
