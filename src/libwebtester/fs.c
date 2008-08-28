/*
 *
 * =============================================================================
 *  file.c
 * =============================================================================
 *
 *  Written (by Nazgul) under GPL
 *
*/

#include "smartinclude.h"
#include "fs.h"
#include "core.h"
#include "macrodef.h"
#include "util.h"

#include <malloc.h>
#include <dirent.h>
#include <string.h>
#include <memory.h>
#include <sys/stat.h>
#include <unistd.h>

#define PET_ANY  0x000
#define PET_FILE 0x001

static int
fs_one                             (const struct dirent *unused)
{
  return 1;
}

dynastruc_t*
dir_listing                        (char *__dir)
{
  struct dirent **eps;
  int i, count;
  dynastruc_t *res=dyna_create ();
  count=scandir (__dir, &eps, fs_one, alphasort);
  for (i=0; i<count; i++)
    {
      if (strcmp (eps[i]->d_name, ".") && strcmp (eps[i]->d_name, ".."))
        dyna_append (res, strdup (eps[i]->d_name), 0);
      free (eps[i]);
    }
  free (eps);

  dyna_sort (res, dyna_sort_listing_comparator);

  return res;
}

static char*
get_full_path_entry                (char *__fn, int __type)
{
  dynastruc_t *paths=core_registered_paths ();
  dyna_item_t *cur;
  char dummy[MAX_PATH_LEN], *data;
  cur=dyna_head (paths);
  struct stat st;

  while (cur)
    {
      data=dyna_data (cur);
      sprintf (dummy, "%s/%s", data, __fn);

      stat (dummy, &st);
      
      if (fexists (dummy))
        {
          if (__type)
            {
              if (__type&PET_FILE && S_ISREG (st.st_mode))
                return strdup (dummy);
            } else
              return strdup (dummy);
        }
      cur=dyna_next (cur);
    }
  return strdup (__fn);
}

char*
get_full_path                      (char *__fn)
{
  return get_full_path_entry (__fn, PET_ANY);
}

char*
get_full_file                      (char *__fn)
{
  return get_full_path_entry (__fn, PET_FILE);
}

size_t
fsize                              (FILE *__stream)
{
  size_t result, pos;
  pos=ftell (__stream);
  fseek (__stream, 0, SEEK_END);
  result=ftell (__stream);
  fseek (__stream, pos, SEEK_SET);
  return result;
}

char*
fload                              (char *__fn)
{
  char *res=0;
  char *fn=get_full_path (__fn);
  size_t size;
  FILE *stream=fopen (fn, "r");
  if (!stream) goto __out;
  size=fsize (stream);
  res=malloc (size+1);
  memset (res, 0, size);
  fread (res, sizeof (char), size, stream);
  res[size]=0;
  fclose (stream);
__out:
  free (fn);
  return res;
}

int
fexists                            (char *__fn)
{
  // Check existing of file
  FILE *ptr;
  ptr = fopen (__fn, "r");
  if (!ptr) return 0;
  fclose (ptr);
  return 1;
}

int
isfile                             (char *__fn)
{
  struct stat st;

  lstat (__fn, &st);
  return S_ISREG (st.st_mode);
}

int
fmkdir                             (char *__dir, mode_t __mode)
{
  // Recursive make directory
  char parent[65536]/*, cur[1024]*/;
  int lastSlash=-1, i, ret;
  for (i=0; i<strlen (__dir); i++) if (__dir[i]=='/') lastSlash=i;
  if (lastSlash>-1)
    {
      for (i=0; i<lastSlash; i++) parent[i]=__dir[i]; parent[i]=0;
      fmkdir (parent, __mode);
    }
  ret=mkdir (__dir, __mode);
  if (!ret)
    chmod (__dir, __mode);

  return 0;
}

int
unlinkdir                          (char *__dir)
{
  // Recursive unlink directory
  struct stat _stat;
  struct dirent **eps;
  long count;
  char dummy[65536];
  count=scandir (__dir, &eps, fs_one, alphasort);
  if (count>=0)
    {
      long i;
      for (i=0; i<count; i++)
        {
          if (strcmp (eps[i]->d_name, ".")&&strcmp (eps[i]->d_name, ".."))
            {
              sprintf (dummy, "%s/%s", __dir, eps[i]->d_name);
              stat (dummy, &_stat);
              if (_stat.st_mode&S_IFDIR)
                {
                  unlinkdir (dummy);
                  rmdir (dummy);
                } else unlink (dummy);
            }
          free (eps[i]);
        }
      free (eps);
      rmdir (__dir);
    }
  return 0;
}

int
copyfile                           (char *__src, char *__dst)
{
  FILE *istream, *ostream;
  size_t size, copied=0, read;
  char buf[FS_COPY_BUF_SIZE+1];
  struct stat file_stat;

  istream=fopen (__src, "r");
  if (!istream)
    return -1;
  ostream=fopen (__dst, "w");
  if (!ostream)
    {
      fclose (istream);
      return -1;
    }
  
  stat (__src, &file_stat);
  
  size=fsize (istream);
  
  while (copied<size)
    {
      read=MIN (FS_COPY_BUF_SIZE, size-copied);
      fread (buf, 1, read, istream);
      fwrite (buf, 1, read, ostream);
      copied+=read;
    }
  
  fclose (istream);
  fclose (ostream);
  
  chmod (__dst, file_stat.st_mode);
  
  return 0;
}

int
fwritebuf                          (char *__fn, char *__buf)
{
  int rc, res=0;
  FILE *stream=fopen (__fn, "w");
  if (!__fn) return -1;
  rc=fwrite (__buf, sizeof (char), strlen (__buf)*sizeof (char), stream);
  if (rc<0)
    res=-1;
  fclose (stream);
  return res;
}

int
fcopydir                           (char *__src, char *__dst)
{
  struct dirent **eps;
  int i, count;
  char full[4096], full_dst[4096];
  struct stat s;

  lstat (__src, &s);
  fmkdir (__dst, STAT_PERMS (s));
  count=scandir (__src, &eps, fs_one, alphasort);
  for (i=0; i<count; i++)
    {
      if (strcmp (eps[i]->d_name, ".") && strcmp (eps[i]->d_name, ".."))
        {
          sprintf (full,     "%s/%s", __src, eps[i]->d_name);
          sprintf (full_dst, "%s/%s", __dst, eps[i]->d_name);
          lstat (full, &s);

          if (S_ISLNK (s.st_mode))
            {
              link (full, full_dst);
            } else
          if (S_ISDIR (s.st_mode))
            {
              fcopydir (full, full_dst);
            } else
          if (S_ISREG (s.st_mode))
            {
              copyfile (full, full_dst);
              chmod (full_dst, STAT_PERMS (s));
            }
        }
      free (eps[i]);
    }
  free (eps);
  return 0;
}


static long
flastid_entry                      (char *__dir, char *__prefix, char *__suffix)
{
  long count;
  struct dirent **eps;
  long id = -1;
  count=scandir (__dir, &eps, fs_one, alphasort);
  if (count>0)
    {
      long i, n;
      char dummy [65535];
      sprintf (dummy, "%s.%%ld.%s", __prefix, __suffix);
      for (i=0; i<count; i++)
        {
          if (sscanf (eps[i]->d_name, dummy, &n))
            if (n>id) id = n;
          free (eps[i]);
        }
      free (eps);
      sprintf (dummy, "%s/%s%ld%s", __dir, __prefix, id, __suffix);
     if (fexists (dummy)) id++;
    }
  return id;
}

static void
fdup_delete_iterator               (char *__dir, char *__pref, long __id, char *__suff)
{
  char dummy[4096];
  sprintf (dummy, "%s/%s.%ld.%s", __dir, __pref, __id, __suff);
  unlink (dummy);
}

static void
fdup_move_down_iterator            (char *__dir, char *__pref, long __id, char *__suff)
{
  char src[4096], dst[4096];
  sprintf (src, "%s/%s.%ld.%s", __dir, __pref, __id, __suff);
  sprintf (dst, "%s/%s.%ld.%s", __dir, __pref, __id-1, __suff);

  rename (src, dst);
}

int             // Make new itered file name
fdup                               (char *__fn, char *__out, char *__add_ext, int __count)
{
  long id;
  char dir[4096], ext[1024], fn[1024], real_ext[1024], dummy[1024];
  
  dirname (__fn, dir);
  fname (__fn, fn);
  dropextension (fn, fn);
  getextension (__fn, ext);
  
  strcpy (real_ext, ext);
  strcat (ext, __add_ext);

  id=flastid_entry (dir, fn, ext);

  if (id>=__count-1)
    {
      int i;
      fdup_delete_iterator (dir, fn, 0, ext);

      for (i=1; i<=__count; i++)
        {
          fdup_move_down_iterator (dir, fn, i, ext);
          fdup_delete_iterator (dir, fn, i, ext);
        }
    } else id++;

  sprintf (dummy, "%s/%s.%ld.%s", dir, fn, id, real_ext);
  copyfile (__fn, dummy);

  if (__out)
    strcpy (__out, dummy);

  return 0;
}
