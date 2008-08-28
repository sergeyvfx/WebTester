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

#include <string.h>
#include <unistd.h>

typedef int (*unpack_proc_t) (char *__fn, char *__dstdir);

typedef struct {
  unpack_proc_t handler;
  char *ext;
} unpacker_info_t;

typedef int (*pack_proc_t)   (char *__fn, char *__dstdir);

typedef struct {
  unpack_proc_t handler;
  char *ext;
} packer_info_t;

////////
// Macroses

#define PREPARSE() \
  char file[4096], dir[4096]; \
  prepare_cmdarg (__file, file); \
  prepare_cmdarg (__dir, dir);

////////
//

static int UnpackBZ2   (char *__file, char *__dir);
static int UnpackTAR   (char *__file, char *__dir);
static int UnpackZIP   (char *__file, char *__dir);
static int UnpackRAR   (char *__file, char *__dir);
static int UnpackGZ    (char *__file, char *__dir);

static int PackBZ2     (char *__file, char *__dir);

////////
//

static unpacker_info_t unpackers_info[] = 
{
  {UnpackBZ2, "bz2"},
  {UnpackTAR, "tar"},
  {UnpackZIP, "zip"},
  {UnpackRAR, "rar"},
  {UnpackGZ,   "gz"},
  {0, 0}
};

static packer_info_t packers_info[] = 
{
  {PackBZ2, "bz2"},
  {0, 0}
};

////////
// Unpack handlers

static int
UnpackBZ2                         (char *__file, char *__dir)
{
  PREPARSE ();
  sys_launch ("bzip2 -f -d %s", file);
  return 0;
}

static int
UnpackTAR                         (char *__file, char *__dir)
{
  PREPARSE ();
  sys_launch ("tar --overwrite --directory=%s -xvvf %s", dir, file);
  return 0;
}

static int
UnpackZIP                         (char *__file, char *__dir)
{
  PREPARSE ();
  sys_launch ("unzip -o %s -d %s", file, dir);
  return 0;
}

static int
UnpackRAR                         (char *__file, char *__dir)
{
  PREPARSE ();
  sys_launch ("unrar x %s %s", file, __dir);
  return 0;
}

static int
UnpackGZ                          (char *__file, char *__dir)
{
  PREPARSE ();
  sys_launch ("gzip -f -d %s", file);
  return 0;
}

////////////////////////////////////////
// Pack handlers

static int
PackBZ2                            (char *__file, char *__dir)
{
  PREPARSE ();
  sys_launch ("bzip2 %s", file);
  return 0;
}

////////////////////////////////////////
//

static int
unpack_file_iter                   (char *__fn, char *__dstdir)
{
  int i=0;
  char ext[128];
  getextension (__fn, ext);
  strlowr (ext, ext);
  while (unpackers_info[i].ext)
    {
      if (!strcmp (unpackers_info[i].ext, ext))
        {
          if (unpackers_info[i].handler)
            return unpackers_info[i].handler (__fn, __dstdir); else
            return -1;
        }
      i++;
    }
  return 1;
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
  int i=0;
  char ext[128];
  strlowr (__packer, ext);
  while (packers_info[i].ext)
    {
      if (!strcmp (packers_info[i].ext, ext))
        {
          if (packers_info[i].handler)
            return packers_info[i].handler (__fn, ""); else
            return -1;
        }
      i++;
    }
  return -1;
}
