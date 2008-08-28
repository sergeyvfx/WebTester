/*
 *
 * ================================================================================
 *  regexp.c
 * ================================================================================
 *
 *  Regular Expressions stuu
 *  Based on the GNU's PCRE
 *
 *  Written (by Nazgul) under General Public License.
 *
*/


#include "regexp.h"
#include "smartinclude.h"
#include "strlib.h"
#include <pcre.h>
#include <string.h>
#include <malloc.h>

static regexp_modifer_t modifers[]={
//  {'', PCRE_DOLLAR_ENDONLY},  // $ matches only at end
//  {'', PCRE_EXTRA},           // strict escape parsing
//  {'', PCRE_UTF8},            // handles UTF8 chars
//  {'', PCRE_UNGREEDY},        // reverses * and *?
//  {'', PCRE_NO_AUTO_CAPTURE}, // disables capturing parens

  {'i', PCRE_CASELESS},       // case insensitive match
  {'m', PCRE_MULTILINE},      // multiple lines match
  {'s', PCRE_DOTALL},         // dot matches newlines
  {'x', PCRE_EXTENDED},       // ignore whitespaces
  {0,0}
};

static regexp_modifer_t customModifers[]={
  {'g', REGEXP_REPLACE_GLOBAL},
  {0,0}
};

////////////////////////////////////////
// BUILT-IN

static char*
preg_replace_parser_iterator       (char *__data, char **__token, int *__flags, int *__errno)
{
  int len=0;
  char c;
  *__flags=0;
  *__errno=ERR_OK;
  
  if (!__data||!*__data) return 0;
  for (;;)
    {
      c=*__data;
      if (!c) break;
      if (c=='$')
        {
          // Numbered string
          if (len) break; // Token is not empty - return it for correct replacing
          *__flags|=PF_NAMED_STRING;
          __data++;
          for (;;)
            {
              c=*__data;
              if (!c||!(c>='0'&&c<='9')) break;
              *(*__token+len++)=c;
              __data++;
            }
          if (len==0) *__errno|=PE_INVALID_NAMED_STRING;
          break;
        } else
      if (c=='\\')
        {
          // Escape character
          if (__data[1]=='n') *(*__token+len++)='\n'; else
          if (__data[1]=='r') *(*__token+len++)='\r'; else
          if (__data[1]=='t') *(*__token+len++)='\t'; else
            *(*__token+len++)=__data[1];
          __data++;
        } else *(*__token+len++)=c;
      __data++;
    }
  *(*__token+len++)=0;
  return __data;
}

int
regexp_free                        (regexp_t *__self)
{
  if (!__self||!__self->handle) return -1;
  free (__self->handle);
  return 0;
}

static int
get_regexp_modifer_code            (char ch, regexp_modifer_t *__modifers)
{
  int i=0;
  while (__modifers[i].ch)
    {
      if (__modifers[i].ch==ch) return __modifers[i].code;
      i++;
    }
  return -1;
}

int
parse_regexp                       (const char *__s, char **__regexp, int *__options, int *__customOptions)
{
  int len=0, code;
  char c;
  int regexpOpened=0;
  int regexpClosed=0;

  *__options=0;
  *__customOptions=0;

  // Getting da regexp
  for (;;)
    {
      c=*__s;
      if (!c) break;
      if (c=='/')
        {
          if (!regexpOpened)
            {
              if (len!=0) return -1; // Trying to open regexp from non-zero character
              regexpOpened=1;
            } else
            {
              // Caught the closing of regexp
              regexpClosed=1; // regexp is seccessfully closed
              __s++; // Go to the first modifer and stop getting regexp
              break;
            }
        } else *(*__regexp+len++)=c;
      __s++;
    }
  if (!regexpClosed) return -1; // Abnormal closing of regexp

  // Getting da modifers
  for (;;)
    {
      c=*__s;
      if (!c) break;
      // Check for standart modifers
      code=get_regexp_modifer_code (c, modifers);
      if (code>0) *__options|=code; else
        {
          code=get_regexp_modifer_code (c, customModifers);
          if (code<=0) return -1;
          *__customOptions|=code;
        }
      __s++;
    }
  *(*__regexp+len)=0;
  return 0;
}

regexp_t
prepare_regexp                     (const char *__regexp, char *__error_message, int *__error_offset)
{
  regexp_t result;
  int errorOffset, options;
  const char *errmsg;
  char *parsedRegexp=malloc (strlen (__regexp));
  result.handle=0;
  if (!parse_regexp (__regexp, &parsedRegexp, &options, &result.userFlags))
    {
      pcre *re=pcre_compile (parsedRegexp, options, &errmsg, &errorOffset, NULL);
      if (!re)
        {
          result.flags=options;
          if (__error_message) strcpy (__error_message, errmsg);
          if (__error_offset) *__error_offset=errorOffset;
        }
      result.handle=re;
    }
  free (parsedRegexp);
  return result;
}

int
regexp_get_vector                  (regexp_t *__re, const char *__str, int *__ovector, int __ovector_size)
{
  if (!__re) return 0;
  return pcre_exec (__re->handle, NULL, __str, strlen(__str), 0, 0, __ovector, __ovector_size);
}

int
match_regexp                       (regexp_t *__re, const char *__str)
{
  const unsigned ovector_size = REGEXP_MAX_VECTOR_SIZE;
  int ovector[ovector_size];
  if (regexp_get_vector (__re, __str, ovector, ovector_size)<=0)
    return -1;
  return 0;
}

int
preg_match                         (const char *__regexp, const char *__str)
{
  int dummy;
  regexp_t re;
  re=prepare_regexp (__regexp, 0, 0);
  if (!re.handle) return -1;
  dummy=match_regexp (&re, __str);
  regexp_free (&re);
  return (!dummy)?(1):(0); // match_regexp returns `0` when string matches to regexp
}

static char*
regexp_replace_iterator            (regexp_t *__re, const char *__mask, const char *__s, int *__l, int *__r)
{
  int i,j,len,substring_count=0,vectorCount;
  char *token, *shift, *append;
  char *out=0;
  char **substrings=0;
  int curAllocSize=0, curLen=0;
  const unsigned ovector_size=REGEXP_MAX_VECTOR_SIZE;
  int ovector[ovector_size];
  int flags, errno, tokenLen;
  *__l=*__r=0;
  // Get the vector
  if ((vectorCount=regexp_get_vector (__re, __s, ovector, ovector_size))<=0) return 0;
  token=malloc (REGEXP_MAX_SUBSTRING_LEN);
  for (i=0; i<vectorCount; i++)
    {
      j=ovector[i*2]; len=0;
      // Get the numbered substring
      while (j<ovector[i*2+1])
        {
          token[len++]=__s[j];
          j++;
        }
      token[len++]=0;
      strarr_append (&substrings, token, &substring_count);
    }
  shift=(char*)__mask;
  out=malloc (REGEXP_START_ALLOCATION_LEN);
  curAllocSize=REGEXP_START_ALLOCATION_LEN;
  strcpy (out, "");
  while ((shift=preg_replace_parser_iterator (shift, &token, &flags, &errno)))
    {
      if (errno==ERR_OK)
        {
          if (flags&PF_NAMED_STRING)
            {
              int index=atoi (token);
              if (index>=substring_count)
                append=""; else
                append=substrings[index];
            } else append=token;
          tokenLen=strlen (append);
          if (curLen+tokenLen>=curAllocSize) out=realloc_string (out, REGEXP_ALLOCATION_DELTA);
          strcat (out, append);
          curLen+=tokenLen;
        } else goto __error_;
    }
  free (token);
  strarr_free (substrings, substring_count);
  *__l=ovector[0]; *__r=ovector[1];
  return out;
__error_:
  free (token);
  strarr_free (substrings, substring_count);
  return 0;
}

char*
regexp_replace                     (regexp_t *__re, const char *__mask, const char *__s)
{
  char *shift=(char*)__s;
  char *dummy, *prefix, *buf=0;
  int l,r,prefixLen,len;

  for (;;)
    {
      dummy=regexp_replace_iterator (__re, __mask, shift, &l, &r);
      len=0;
      if (dummy) len=strlen (dummy);
      //len=(dummy)?(strlen (dummy)):(0);
      prefixLen=l;
      if (prefixLen>0)
        {
          prefix=malloc (prefixLen+1);
          strsubstr (shift, 0, prefixLen, prefix);
        } else prefix="";
      if (prefixLen+len)
        {
          buf=realloc_string (buf, prefixLen+len);
          strcat (buf, prefix);  // append to buffer new prefix
          if (dummy)
            {
              strcat (buf, dummy); // append replaced string
              free (dummy);        // Free replaced buffer
            }
        }
      if (prefixLen>0) free (prefix);
      if (!dummy) break;
      shift+=r;
      if (!r) break;
      if (!__re->userFlags&REGEXP_REPLACE_GLOBAL) break;
    }
  buf=realloc_string (buf, strlen (shift));
  strcat (buf, shift);
  return buf;
}

void
preg_replace                       (const char *__regexp, const char *__mask, char *__str)
{
  char *result;
  regexp_t re;
  re=prepare_regexp (__regexp, 0, 0);
  if (!re.handle) return;
  result=regexp_replace (&re, __mask, __str);
  strcpy (__str, result);
  regexp_free (&re);
  free (result);
}
