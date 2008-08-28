/*
 *
 * ================================================================================
 *  strlib.c
 * ================================================================================
 *
 *  String library.  Some powerfull functions to work with string.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "strlib.h"
#include "smartinclude.h"
#include <malloc.h>
#include <string.h>

void
strupr                             (char *__src, char *__dest)
{
  int len=0;
  while (__src && *__src)
    {
      if ((*__src>='a')&&(*__src<='z'))
        *(__dest+len++)=*__src-('a'-'A'); else
      *(__dest+len++)=*__src;
      __src++;
    }
  *(__dest+len)=0;
}

void
strlowr                            (char *__src, char *__dest)
{
  int len=0;
  while (__src && *__src)
    {
      if ((*__src>='A')&&(*__src<='Z'))
        *(__dest+len++)=*__src+('a'-'A'); else
      *(__dest+len++)=*__src;
      __src++;
    }
  *(__dest+len)=0;
}

long
explode                            (char *__s, char *__separator, char ***__out)
{
  char *token=malloc (4096);
  int len=0, i, n=strlen (__s), j, n2=strlen (__separator);
  int pr, uk=0;
  i=0;
  (*__out)=0;
  while (i<n)
    {
      pr=0;
      for (j=0; j<n2; j++)
        if (__s[i+j]!=__separator[j]) pr=1;
      if (pr)
        *(token+len++)=__s[i]; else
        {
          *(token+len++)=0;
          strarr_append (__out, token, &uk);
          len=0;
        }
      i++;
    }
  if (strcmp (token, __separator))    
    {
      *(token+len++)=0;
      strarr_append (__out, token, &uk);
    }
  strarr_append (__out, 0, &uk);
  free (token);
  return uk-1;
}

void
free_explode_data                  (char **__self)
{
  int i=0;
  while (__self[i])
    free (__self[i++]);
  free (__self);
}

int
strlastchar                        (char *__str, char __ch)
{
  int i=0, n=strlen (__str), last=-1;
  for (i=0; i<n; i++) if (__str[i]==__ch) last=i;
  return last;
}

void
strsubstr                          (char *__src, int __start, int __len, char *__out)
{
  int i=0;
  while (i<__len)
    {
      __out[i]=__src[i+__start];
      i++;
    }
  __out[i]=0;
}

void
addslashes                         (char *__self, char *__out)
{
  int i=0, n, len=0;
  n=strlen (__self);
  while (i<n)
    {
      if (__self[i]==' ') __out[len++]='\\';
      if (__self[i]=='"') __out[len++]='\\';
      if (__self[i]=='\'') __out[len++]='\\';
      __out[len++]=__self[i];
      i++;
    }
  __out[len]=0;
}

void
stripslashes                       (char *__self, char *__out)
{
  int i=0, n, len=0;
  n=strlen (__self);
  while (i<n)
    {
      if (__self[i]=='\\')
        {
          i++;
          if (__self[i]=='n') __out[len++]='\n'; else
          if (__self[i]=='t') __out[len++]='\t'; else
          if (__self[i]=='r') __out[len++]='\r'; else
          __out[len++]=__self[i];
        } else  __out[len++]=__self[i];
      i++;
    }
  __out[len]=0;
}

void
strarr_append                      (char ***__arr, char *__s, int *__count)
{
  int i;
  char **newArr=malloc ((*__count+1)*sizeof (char*));
  for (i=0; i<*__count; i++) newArr[i]=(*__arr)[i];
  if (__s)
    newArr[*__count]=strdup (__s); else
    newArr[*__count]=0;
  (*__count)++;
  SAFE_FREE (*__arr);
  (*__arr)=newArr;
}

void
strarr_free                        (char **__arr, int __count)
{
  int i;
  if (!__arr) return;
  for (i=0; i<__count; i++) SAFE_FREE (__arr[i]);
  free (__arr);
}

char*
realloc_string                     (char *__s, int __delta)
{
  char *ptr;
  int len;
  __delta++; // For zero-ending
  if (!__s)
    {
      if (__delta>0)
      {
        char *ptr=malloc (__delta);
        memset (ptr, 0, __delta);
        return ptr;
      }
      return 0;
    }
  len=strlen (__s);
  if (len+__delta<=0) return 0;
  ptr=malloc (len+__delta);
  memset (ptr, 0, len+__delta);
  strncpy (ptr, __s, MIN(len+__delta,len));
  free (__s);
  return ptr;
}

void
trim                               (char *__data, char *__out)
{
  int first=0, last=strlen (__data)-1, len=0, i;
  while (__data[first]<=32 && first<=last)
    first++;
  while (__data[last]<=32 && first<=last)
    last--;
  
  for (i=first; i<=last; i++)
    __out[len++]=__data[i];
  __out[len]=0;
}
