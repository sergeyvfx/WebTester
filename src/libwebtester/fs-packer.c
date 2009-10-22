/**
 * WebTester Server - server of on-line testing system
 *
 * Packing stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "fs.h"
#include "util.h"
#include "strlib.h"
#include "conf.h"
#include "regexp.h"

#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define VAR_REPLACE(__str,__var, __val) \
  { \
    char *s = preg_replace ("/\\$\\{" __var "\\}/gs", __str, __val); \
    strcpy (__str, s); \
    free (s); \
  }

/**
 * Iterator of unpack_file()
 *
 * @param __fn - name of file to unpack
 * @param __dstdir - destination dir
 * @return non-zero if file is not unpackable, zero otherwise
 */
static int
unpack_file_iter (const char *__fn, const char *__dstdir)
{
  char ext[128], dummy[4096], tpl[4096] = {0};
  getextension (__fn, ext);
  strlowr (ext, ext);

  snprintf (dummy, BUF_SIZE (dummy), "CORE/Unpackers/%s", ext);
  CONFIG_PCHAR_KEY (tpl, dummy);

  if (!strcmp (tpl, ""))
    {
      return 1;
    }

  VAR_REPLACE (tpl, "file", __fn);
  VAR_REPLACE (tpl, "dstdir", __dstdir);

  sys_launch (tpl);

  return 0;
}

/**
 * Unpack file
 *
 * @param __fn - name of file to unpack
 * @param __dstdir  - destination directory
 * @return zero on success, non-zero otherwise
 */
int
unpack_file (const char *__fn, const char *__dstdir)
{
  char fn[4096];
  int dotcount = 0, i, n = strlen (__fn), tmp;

  strcpy (fn, __fn);

  for (i = n - 1; i >= 0; i--)
    {
      if (fn[i] == '/') break;
      if (fn[i] == '.') dotcount++;
    }

  while (dotcount)
    {
      tmp = unpack_file_iter (fn, __dstdir);

      if (tmp < 0)
        {
          return -1;
        }
      else
        {
          if (tmp > 0) return 0;
        }

      if (dotcount > 1)
        {
          for (i = n - 1; i >= 0; i--)
            {
              if (fn[i] == '.')
                {
                  fn[i] = 0;
                  n = i + 1;
                  break;
                }
            }
        }
      dotcount--;
    }

  unlink (fn);

  return 0;
}

/**
 * Pack file
 *
 * @param __fn - name of file to pack
 * @param __packer - name of packer to use
 * @return zero om success, non-zero otherwise
 */
int
pack_file (const char *__fn, const char *__packer)
{
  char dummy[4096], tpl[4096] = {0}, packer[1024];
  strlowr (__packer, packer);

  snprintf (dummy, BUF_SIZE (dummy), "CORE/Packers/%s", packer);
  CONFIG_PCHAR_KEY (tpl, dummy);

  if (!strcmp (tpl, ""))
    {
      return -1;
    }

  VAR_REPLACE (tpl, "file", __fn);

  sys_launch (tpl);

  return 0;
}
