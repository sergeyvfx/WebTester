/**
 * WebTester Server - server of on-line testing system
 *
 * Implementation of assaciave arrays
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "assarr.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Calculate hash-function of key
 *
 * @param __self - descriptor of array for which key will be belong
 * @param __str - key for which hash will be calculated
 * @reutrn value of hash function
 */
static long
assarr_hash (assarr_t *__self, const char *__str)
{
  long h = 0;
  long i;
  for (i = 0; i < (long) strlen (__str); i++)
    {
      h = (h * __self->k + __str[i]) % __self->m;
    }
  return h;
}

/**
 * Define element of array
 *
 * @param __self - descriptor of array
 * @param __key - key of element to define
 * @return zero on success, non-zero otherwise
 */
static int
assarr_define (assarr_t *__self, const char *__key)
{
  assarr_entry_t *cur, *prev, *ptr;
  long hash;
  hash = assarr_hash (__self, __key);
  ptr = malloc (sizeof (assarr_entry_t));
  ptr->key = strdup (__key);
  ptr->value = 0;
  ptr->next_ptr = 0;
  __self->count++;
  if (__self->data[hash] == 0)
    {
      __self->data[hash] = ptr;
    }
  else
    {
      cur = __self->data[hash];
      prev = 0;
      while (cur)
        {
          prev = cur;
          cur = cur->next_ptr;
        }
      prev->next_ptr = ptr;
    }
  return 0;
}

/**
 * Return entry of associative array by it's key
 *
 * @param __self - descriptor of array
 * @param __key - key of entry to get
 * @return entry descriptor, or NULL if it not fount
 */
static assarr_entry_t*
assarr_get_entry (assarr_t *__self, const char *__key)
{
  assarr_entry_t *cur;
  long hash;
  if (!__self || !__key) return 0;
  hash = assarr_hash (__self, __key);
  cur = __self->data[hash];
  while (cur)
    {
      if (!strcmp (cur->key, __key))
        {
          /* Entry has been found */
          return cur;
        }
      cur = cur->next_ptr;
    }

  /* There is no such entry */
  return NULL;
}

/********
 * User's backend
 */

/**
 * Create new assaciative array
 *
 * @return new associative array
 */
assarr_t*
assarr_create (void)
{
  long i;
  long __k = 43; /* Most useful constants */
  long __m = 10987;
  assarr_t *ptr;
  ptr = malloc (sizeof (assarr_t));
  ptr->m = __m;
  ptr->k = __k;
  ptr->data = malloc ((__m + 1) * sizeof (assarr_entry_t));
  ptr->count = 0;
  for (i = 0; i < __m; i++)
    {
      ptr->data[i] = 0;
    }
  return ptr;
}

/**
 * Destroy associative array
 *
 * @param __self - array to be destroyed
 * @param __deter - deleter of elements
 */
void
assarr_destroy (assarr_t *__self, assarr_deleter __deleter)
{
  long i;
  assarr_entry_t *cur, *tmp;
  for (i = 0; i < __self->m; i++)
    {
      cur = __self->data[i];
      while (cur)
        {
          if (__deleter) __deleter (cur->value);
          free (cur->key);
          tmp = cur;
          cur = cur->next_ptr;
          free (tmp);
        }
      __self->data[i] = 0;
    }
  free (__self->data);
  free (__self);
}

/**
 * Deleter of element of assaciative array
 * Calls free() for each element
 *
 * @param __item - item to destroy
 */
void
assarr_deleter_free_ref_data (void *__item)
{
  free (__item);
}

/**
 * Get value by key
 *
 * @param __self - descriptor of array
 * @param __key - key of element to get
 * @return value stored in element with specified key.
 * NULL if there is no such element.
 */
void*
assarr_get_value (assarr_t *__self, const char *__key)
{
  assarr_entry_t *entry;
  entry = assarr_get_entry (__self, __key);

  if (entry)
    {
      return entry->value;
    }

  return NULL;
}

/**
 * Check if element is set
 *
 * @param __self - descriptor of array
 * @param __key - key of element to check
 * @return zero if element is unset, non-zero otherwise
 */
int
assarr_isset (assarr_t *__self, const char *__key)
{
  if (assarr_get_entry (__self, __key))
    {
      return 1;
    }

  return 0;
}

/**
 * Set value by key
 *
 * @param __self - descriptor of array
 * @param __key - key of element
 * @param __value - value of element
 */
void
assarr_set_value (assarr_t *__self, const char *__key, void *__value)
{
  assarr_entry_t *entry;

  if (!assarr_isset (__self, __key))
    {
      /* If there is no element with such key */
      /* we should define it */
      assarr_define (__self, __key);
    }

  entry = assarr_get_entry (__self, __key);

  /*
   * FIXME: Need to call deleter for previous value
   */

  entry->value = __value;
}

/**
 * Unset value by key
 *
 * @param __self - descriptor of array
 * @param __key - key of element to onset
 * @param __deleter - deleter of data
 * @return zero on success, non-zero otherwise
 */
int
assarr_unset_value (assarr_t *__self, const char *__key,
                    assarr_deleter __deleter)
{
  long hash;
  assarr_entry_t *cur, *prev;

  if (!assarr_isset (__self, __key))
    {
      /* There is no such element */
      return -1;
    }

  hash = assarr_hash (__self, __key);
  cur = __self->data[hash];
  prev = 0;

  /* Seach needed entry and store it's parent */
  while (cur)
    {
      if (!strcmp (cur->key, __key)) break;
      prev = cur;
      cur = cur->next_ptr;
    }

  if (prev)
    {
      prev->next_ptr = cur->next_ptr;
    }
  else
    {
      __self->data[hash] = cur->next_ptr;
    }

  if (__deleter)
    {
      /* Destroy data */
      __deleter (cur->value);
    }

  free (cur);
  return 0;
}

/**
 * Unset all values
 *
 * @param __self - descriptor of array
 * @param __deleter - deletr of data
 * @return zero on success, non-zero otherwise
 */
int
assarr_unset_all (assarr_t *__self, assarr_deleter __deleter)
{
  char *k;
  void *v;
  if (!__self)
    {
      return -1;
    }

  ASSARR_FOREACH_DO (__self, k, v);
    assarr_unset_value (__self, k, __deleter);
  ASSARR_FOREACH_DONE

  return 0;
}

// Pack assaciative array to string

/**
 * Pack ass. array PCHAR data to string
 * !!! WARNING !!! Works ONLY with PCHAR data
 *
 * @param __self - descripto of array
 * @param __out - result buffer
 */
void
assarr_pack (assarr_t *__self, char **__out)
{
  char buf[1024];
  char *key;
  void *value;
  strcpy (*__out, "");
  ASSARR_FOREACH_DO (__self, key, value);
    strcat (*__out, key);
    strcat (*__out, ";");
    sprintf (buf, "%ld", (long) strlen ((char*) value));
    strcat (*__out, buf);
    strcat (*__out, ";");
    strcat (*__out, (char*) value);
    strcat (*__out, ";");
  ASSARR_FOREACH_DONE;
}

/**
 * Unpack string to assaciative array
 *
 * @param __data - array, packed to string
 * @param __arr - unpacked array
 */
void
assarr_unpack (const char *__data, assarr_t *__arr)
{
  long i = 0, n = (long) strlen (__data);
  char *token;
  char *key = 0, *value = 0;
  long len, data_len = 0;
  int state = 0;
  if (!__data)
    {
      return;
    }
  token = malloc (n + 1);
  while (i < n)
    {
      if (__data[i] == ';')
        {
          /* Token is ready */
          if (state == 0)
            {
              /* Remember key name */
              key = malloc (strlen (token) + 1);
              strcpy (key, token);
              state = 1;
              strcpy (token, "");
              len = 0;
            }
          else
            if (state == 1)
            {
              /* Rememer value's length */
              data_len = atol (token);
              state = 2;
              strcpy (token, "");
              len = 0;
            }
          else
            if (state == 2)
            {
              /* Create new pchar-value and add it to array */
              value = malloc (strlen (token) + 1);
              strcpy (value, token);
              assarr_set_value (__arr, key, value);
              free (key);
              key = 0;
              state = 0;
              strcpy (token, "");
              len = 0;
            }
        }
      else
        if (state != 2)
        {
          /* Get token until semicomon */
          len = 0;
          while (__data[i] != ';' && i < n)
            {
              *(token + len++) = __data[i++];
            }
          *(token + len) = 0;
          i--;
        }
      else
        {
          /* Get value with known length */
          len = 0;
          while (len != data_len && i < n)
            {
              *(token + len++) = __data[i++];
            }
          *(token + len) = 0;
          i--;
        }
      i++;
    }
  SAFE_FREE (key);
  free (token);
}

/**
 * Get count of elements in array
 *
 * @param __self - descriptor of array
 * @return count of elements
 */
int
assarr_get_count (assarr_t *__self)
{
  return __self->count;
}
