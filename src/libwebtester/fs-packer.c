/*
 *
 * =============================================================================
 *  fs-packer.c
 * =============================================================================
 *
 *  Packing stuff
 *
 *  Written (by Nazgul) under GPL
 *
*/

#include "smartinclude.h"
#include "fs.h"
#include "util.h"
#include "strlib.h"
#include "conf.h"
#include "regexp.h"

#include <string.h>
#include <unistd.h>

////////////////////////////////////////
//

#define VAR_REPLACE(__str,__var, __val) \
  preg_replace ("/\\$\\{" __var "\\}/gs", __val, __str);

///////////////////////////////////////
//

static int
unpack_file_iter                   (char *__fn, char *__dstdir)
{
  char ext[128], dummy[4096], tpl[4096]={0};
  getextension (__fn, ext);
  strlowr (ext, ext);

  sprintf (dummy, "CORE/Unpackers/%s", ext);
  CONFIG_PCHAR_KEY (tpl, dummy);

  if (!strcmp (tpl, ""))
    return 1;

  VAR_REPLACE (tpl,   "file",     __fn);
  VAR_REPLACE (tpl,   "dstdir",  __dstdir);

  sys_launch (tpl);

  return 0;
}

int
unpack_file                        (char *__fn, char *__dstdir)
{
  char fn[4096];
  int dotcount=0, i, n=strlen (__fn), tmp;

  strcpy (fn, __fn); 
  
  for (i=n-1; i>=0; i--)
    {
      if (fn[i]=='/') break;
      if (fn[i]=='.') dotcount++;
    }

  while (dotcount)
    {
      tmp=unpack_file_iter (fn, __dstdir);

      if (tmp<0)
        return -1; else
          if (tmp>0) return 0;

      if (dotcount>1)
        for (i=n-1; i>=0; i--)
          if (fn[i]=='.')
            {
              fn[i]=0;
              n=i+1;
              break;
            }
      dotcount--;
    }

  unlink (fn);
  
  return 0;
}

int
pack_file                          (char *__fn, char *__packer)
{
  char dummy[4096], tpl[4096]={0}, packer[1024];
  strlowr (__packer, packer);

  sprintf (dummy, "CORE/Packers/%s", packer);
  CONFIG_PCHAR_KEY (tpl, dummy);

  if (!strcmp (tpl, ""))
    return -1;

  VAR_REPLACE (tpl,   "file",     __fn);

  sys_launch (tpl);

  return 0;
}
