/**
 * WebTester Server - server of on-line testing system
 *
 * String library. Includes some powerfull
 * functions to work with strings.
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "strlib.h"
#include "smartinclude.h"
#include <malloc.h>
#include <string.h>

/**
 * Convert string to upper case
 *
 * @param __src - source string
 * @param __dest - pointer to destination buffer
 */
void
strupr (const char *__src, char *__dest)
{
  int len = 0;

  while (__src && *__src)
    {
      if ((*__src >= 'a') && (*__src <= 'z'))
        {
          *(__dest + len++) = *__src - ('a' - 'A');
        }
      else
        {
          *(__dest + len++) = *__src;
        }
      __src++;
    }

  *(__dest + len) = 0;
}

/**
 * Convert string to lower case
 *
 * @param __src - source string
 * @param __dest - pointer to destination buffer
 */
void
strlowr (const char *__src, char *__dest)
{
  int len = 0;

  while (__src && *__src)
    {
      if ((*__src >= 'A') && (*__src <= 'Z'))
        {
          *(__dest + len++) = *__src + ('a' - 'A');
        }
      else
        {
          *(__dest + len++) = *__src;
        }

      __src++;
    }

  *(__dest + len) = 0;
}

/**
 * Split string by separator into array of string
 *
 * @param __s - string to split
 * @param __separator - separator
 * @param __out - pointer to array of strings where result will be saved
 ** @return count of elements in array
 */
long
explode (const char *__s, const char *__separator, char ***__out)
{
  char *token = malloc (4096);
  int len = 0, i, n = strlen (__s), j, n2 = strlen (__separator);
  int pr, uk = 0;
  i = 0;
  (*__out) = 0;

  while (i < n)
    {
      pr = 0;
      for (j = 0; j < n2; j++)
        {
          if (__s[i + j] != __separator[j]) pr = 1;
        }

      if (pr)
        {
          *(token + len++) = __s[i];
        }
      else
        {
          *(token + len++) = 0;
          strarr_append (__out, token, &uk);
          len = 0;
        }
      i++;
    }

  if (strcmp (token, __separator))
    {
      *(token + len++) = 0;
      strarr_append (__out, token, &uk);
    }

  strarr_append (__out, 0, &uk);
  free (token);

  return uk - 1;
}

/**
 * Free array from explode
 *
 * @param __self - array to be freed
 */
void
free_explode_data (char **__self)
{
  int i = 0;
  while (__self[i])
    free (__self[i++]);
  free (__self);
}

/**
 * Get last occurence of character in string
 *
 * @param __str - string to scan
 * @param __ch - character to find
 * @return last occurence of character in string or -1 if there is no
 * such character in string
 */
int
strlastchar (const char *__str, char __ch)
{
  int i = 0, n = strlen (__str), last = -1;

  for (i = 0; i < n; i++)
    {
      if (__str[i] == __ch) last = i;
    }

  return last;
}

/**
 * Extract substring from string
 *
 * @param __src - string from which substirng will be extracted
 * @param __start - start character
 * @param __len - length of substring
 * @param __out - pointer to memory where substring will be saved
 */
void
strsubstr (const char *__src, int __start, int __len, char *__out)
{
  int i = 0;

  while (i < __len)
    {
      __out[i] = __src[i + __start];
      i++;
    }

  __out[i] = 0;
}

/**
 * Add slashed before `dangerous` characters
 *
 * @param __self - source string
 * @oaram __out - destination string
 */
void
addslashes (const char *__self, char *__out)
{
  int i = 0, n, len = 0;
  n = strlen (__self);

  while (i < n)
    {
      if (__self[i] == ' ')
        {
          __out[len++] = '\\';
        }
      else if (__self[i] == '"')
        {
          __out[len++] = '\\';
        }
      else if (__self[i] == '\'')
        {
          __out[len++] = '\\';
        }

      __out[len++] = __self[i];

      i++;
    }

  __out[len] = 0;
}

/**
 * Strinp slashes from string
 *
 * @param __self - source string
 * @oaram __out - destination string
 */
void
stripslashes (const char *__self, char *__out)
{
  int i = 0, n, len = 0;
  n = strlen (__self);

  while (i < n)
    {
      if (__self[i] == '\\')
        {
          i++;
          if (__self[i] == 'n')
            {
              __out[len++] = '\n';
            }
          else if (__self[i] == 't')
            {
              __out[len++] = '\t';
            }
          else if (__self[i] == 'r')
            {
              __out[len++] = '\r';
            }
          else
            {
              __out[len++] = __self[i];
            }
        }
      else __out[len++] = __self[i];
      i++;
    }

  __out[len] = 0;
}

/**
 * Append string to array of string
 *
 * @param __arr - pointer to array of string
 * @param __s - stirng to append
 * @param __count - count of items in array
 */
void
strarr_append (char ***__arr, const char *__s, int *__count)
{
  int i;
  char **newArr = malloc ((*__count + 1) * sizeof (char*));

  for (i = 0; i<*__count; i++)
    {
      newArr[i] = (*__arr)[i];

    }
  if (__s)
    {
      newArr[*__count] = strdup (__s);
    }
  else
    {
      newArr[*__count] = NULL;
    }

  (*__count)++;
  SAFE_FREE (*__arr);
  (*__arr) = newArr;
}

/**
 * Free array f string
 *
 * @param __arr - array to be freed
 * @param __count - count of elements in array
 */
void
strarr_free (char **__arr, int __count)
{
  int i;
  if (!__arr)
    {
      return;
    }

  for (i = 0; i < __count; i++)
    {
      SAFE_FREE (__arr[i]);
    }

  free (__arr);
}

/**
 * Realloc memory allocated for string
 *
 * @param __s - string
 * @param __delta - memory delta
 * @return new string
 */
char*
realloc_string (const char *__s, int __delta)
{
  char *ptr;
  int len;

  /* For null-terminator */
  __delta++;

  if (!__s)
    {
      if (__delta > 0)
        {
          char *ptr = malloc (__delta);
          memset (ptr, 0, __delta);
          return ptr;
        }
      return 0;
    }

  len = strlen (__s);

  if (len + __delta <= 0)
    {
      return 0;
    }

  ptr = malloc (len + __delta);
  memset (ptr, 0, len + __delta);
  strncpy (ptr, __s, MIN (len + __delta, len));
  free ((char*)__s);

  return ptr;
}

/**
 * Trim space characters from beginning and ending of string
 *
 * @param __data - source string
 * @oaram __out - output string
 */
void
trim (const char *__data, char *__out)
{
  int first = 0, last = strlen (__data) - 1, len = 0, i;

  while (__data[first] <= 32 && first <= last)
    {
      first++;
    }

  while (__data[last] <= 32 && first <= last)
    {
      last--;
    }

  for (i = first; i <= last; i++)
    {
      __out[len++] = __data[i];
    }
  __out[len] = 0;
}
