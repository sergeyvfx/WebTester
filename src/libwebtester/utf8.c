/*
 *
 * ================================================================================
 *  utf8.c
 * ================================================================================
 *
 *  This module contains utf-8 stuff and some other stuff to work with charsets.
 *  (Based on the source of X MultiMedia Systems)
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "smartinclude.h"
#include <langinfo.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>
#include <malloc.h>

char*
get_current_charset                (void)
{
  char *charset=getenv ("CHARSET");
  if (!charset)
    charset=nl_langinfo (CODESET);
  if (!charset)
    charset="ISO-8859-1";
  return charset;
}

char*
convert_string                     (const char *__string, char *__from, char *__to)
{
  size_t outleft, outsize, length;
  iconv_t cd;
  char *out, *outptr;
  const char *input=(const char *)__string;

  if (!__string) return NULL;

  length=strlen (__string);
	
  if ((cd=iconv_open(__to, __from))==(iconv_t)-1) // Convertation not supported
    return strdup(__string); 

  outsize=((length+3)&~3)+1;
  out=malloc (outsize);
  outleft=outsize-1;
  outptr=out;

 retry:
  if (iconv (cd, (char **__restrict)&input, &length, &outptr, &outleft)==(size_t)-1)
    {
      int used;
      switch (errno)
       {
         case E2BIG:
           used=outptr-out;
           outsize=(outsize-1)*2+1;
           out=realloc (out, outsize);
           outptr=out+used;
           outleft=outsize-1-used;
          goto retry;
        case EINVAL:
          break;
        case EILSEQ:
          // Invalid sequence
          input++;
          length=strlen (input);
          goto retry;
        default:
          break;
       }
    }
  *outptr='\0';
  iconv_close (cd);
  return out;
}

char*
convert_to_utf8                    (const char *__string)
{
	char *charset=get_current_charset ();
	return convert_string (__string, charset, "UTF-8");
}

char*
convert_from_utf8                  (const char *__string)
{
	char *charset=get_current_charset ();
	return convert_string (__string, "UTF-8", charset);
}

char*
recode                             (const char *__string, char *__from, char *__to)
{
	return convert_string (__string, __from, __to);
}
