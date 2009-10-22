/**
 * WebTester Server - server of on-line testing system
 *
 * Dynamic structures stuff (stack, queues, deks).
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "dynastruc.h"
#include "util.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

/**
 * Swapper of two dyna items
 *
 * @param __a, __b - items to be swapped
 */
static void
swap (dyna_item_t *__a, dyna_item_t *__b)
{
  int stag;
  void *sdata;

  stag = __a->tag;
  __a->tag = __b->tag;
  __b->tag = stag;
  sdata = __a->data;
  __a->data = __b->data;
  __b->data = sdata;
}

/**
 * Deleter which frees memory of data
 *
 * @param __self - data to be deleted
 */
void
dyna_deleter_free_ref_data (void *__self)
{
  free (__self);
}

/**
 * Create new dynastruc
 *
 * @return descriptor of new dynastruc, or NULL if failed
 */
dynastruc_t*
dyna_create (void)
{
  dynastruc_t *new_ptr;
  new_ptr = malloc (sizeof (dynastruc_t));
  if (!new_ptr)
    {
      return NULL;
    }
  new_ptr->head = 0;
  new_ptr->tail = 0;
  new_ptr->find_data = 0;
  new_ptr->count = 0;
  return new_ptr;
}

/**
 * Destroy dynastruc
 *
 * @param __self - dynastruc to be destroyed
 * @param __deleter - deleter of elements
 * @return zero on success, non-zero otherwise
 */
int
dyna_destroy (dynastruc_t *__self, dyna_deleter __deleter)
{
  if (!__self)
    {
      /* Trying to work with wrong struct */
      return -1;
    }

  dyna_delete_all (__self, __deleter);
  free (__self);

  return 0;
}

/**
 * Delete element of dynastruc
 *
 * @param __self - descriptor of dynastruc
 * @param __item - item to delete
 * @param __deleter - deleter of item
 * @return zero on success, non-zero otherwise
 */
int
dyna_delete (dynastruc_t *__self, dyna_item_t *__item, dyna_deleter __deleter)
{
  if (!__self)
    {
      /* Trying to work with wrong struct */
      return -1;
    }
  if (!__item)
    {
      /* Trying to delete zero item */
      return -1;
    }

  if (__item->next_ptr)
    {
      ((dyna_item_t*) __item->next_ptr)->prev_ptr = __item->prev_ptr;
    }

  if (__item->prev_ptr)
    {
      ((dyna_item_t*) __item->prev_ptr)->next_ptr = __item->next_ptr;
    }

  if (__item == __self->head)
    {
      __self->head = __item->next_ptr;
    }

  if (__self->tail == __item)
    {
      __self->tail = __item->prev_ptr;
    }

  if (__deleter)
    {
      __deleter (__item->data);
    }

  free (__item);
  __self->count--;
  return 0;
}

/**
 * Delete all items of dynastruc
 *
 * @param __self - descriptor of dynastruc
 * @param __deleter - deleter of items
 * @return zero on success, non-zero otherwise
 */
int
dyna_delete_all (dynastruc_t *__self, dyna_deleter __deleter)
{
  dyna_item_t *current_ptr;
  dyna_item_t *temp_ptr;
  if (!__self)
    {
      /* Trying to work with wrong struct */
      return -1;
    }
  current_ptr = (dyna_item_t*) __self->head;
  while (current_ptr)
    {
      temp_ptr = current_ptr;
      current_ptr = (dyna_item_t*) current_ptr->next_ptr;
      dyna_delete (__self, temp_ptr, __deleter);
    }
  return 0;
}

/**
 * Push element to stack
 *
 * @param __self - descriptor of dynastruc
 * @param __data - data of element
 * @param __tag - tag of element
 * @return zero on success, non-zero otherwise
 */
int
dyna_push (dynastruc_t *__self, void *__data, int __tag)
{
  dyna_item_t *new_ptr;
  if (!__self)
    {
      /* Trying to work with wrong struct */
      return -1;
    }
  new_ptr = malloc (sizeof (dyna_item_t));
  if (!new_ptr)
    {
      /* FATAL ERROR: Memory has not been allocated */
      return -1;
    }
  new_ptr->data = __data;
  new_ptr->tag = __tag;
  new_ptr->prev_ptr = 0;
  new_ptr->next_ptr = __self->head;
  if (__self->head) __self->head->prev_ptr = (void*) new_ptr;
  __self->head = new_ptr;
  if (!__self->tail) __self->tail = __self->head;
  __self->count++;
  return 0;
}

/**
 * Pop element from stack
 *
 * @param __self - descriptor of dynastruc
 * @param __data - returned data
 * @param __tag - returned tag
 * @return zero on success, non-zero otherwise
 */
int
dyna_pop (dynastruc_t *__self, void **__data, int *__tag)
{
  dyna_item_t *temp_ptr;
  if (!__self)
    {
      /* Trying to work with wrong struct */
      return -1;
    }

  if (__self->count == 0)
    {
      return -1;
    }

  if (__data)
    {
      *__data = __self->head->data;
    }

  if (__tag)
    {
      *__tag = __self->head->tag;
    }

  temp_ptr = __self->head;
  __self->head = (dyna_item_t*) __self->head->next_ptr;
  dyna_delete (__self, temp_ptr, 0);

  if (!__self->head)
    {
      __self->tail = NULL;
    }
  return 0;
}

/**
 * Start new search
 *
 * @param __self - descriptor of dynastruc
 * @return zero on success, non-zero otherwise
 */
int
dyna_search_reset (dynastruc_t *__self)
{
  if (!__self)
    {
      /* Trying to work with wrong struct */
      return -1;
    }
  __self->find_data = 0;
  return 0;
}

/**
 * Search for element
 *
 * @param __self - descriptor of dynastruc
 * @param __data - data to search
 * @param __tag - tag t osearch
 * @param __comparator - comparator of two items
 * @return descriptor of dynastruc item, or NULL if there is no such item
 */
dyna_item_t*
dyna_search (dynastruc_t *__self, void *__data, int __tag,
             dyna_comparator __comparator)
{
  if (!__self)
    {
      /* Trying to work with wrong struct */
      return 0;
    }

  if (!__self->find_data)
    {
      __self->find_data = __self->head;
    }
  else
    {
      __self->find_data = __self->find_data->next_ptr;
    }

  while (__self->find_data)
    {
      if (__self->find_data->tag == __tag)
        {
          if (__comparator)
            {
              if (__comparator (__self->find_data->data, __data))
                {
                  return __self->find_data;
                }
            }
          else
            {
              return __self->find_data;
            }
        }
      __self->find_data = __self->find_data->next_ptr;
    }
  return 0;
}

/**
 * Add element to front of list
 *
 * @param __self - descriptor of dynastruc
 * @param __data - data of element
 * @param __tag - tag of element
 * @return zero on success, non-zero otherwise
 */
int
dyna_add_to_front (dynastruc_t *__self, void *__data, int __tag)
{
  return dyna_push (__self, __data, __tag);
}

/**
 * Add element to back of list
 *
 * @param __self - descriptor of dynastruc
 * @param __data - data of element
 * @param __tag - tag of element
 * @return zero on success, non-zero otherwise
 */
int
dyna_add_to_back (dynastruc_t *__self, void *__data, int __tag)
{
  dyna_item_t *new_ptr;
  if (!__self)
    {
      /* Trying to work with wrong struct */
      return -1;
    }
  new_ptr = malloc (sizeof (dyna_item_t));
  if (!new_ptr)
    {
      /* FATAL ERROR: Memory has not been allocated */
      return -1;
    }

  new_ptr->data = __data;
  new_ptr->tag = __tag;
  new_ptr->prev_ptr = __self->tail;
  new_ptr->next_ptr = 0;
  if (__self->tail)
    {
      __self->tail->next_ptr = new_ptr;
      __self->tail = new_ptr;
    }
  else
    {
      __self->tail = __self->head = new_ptr;
    }
  __self->count++;
  return 0;
}

/**
 * Delete element from front of list
 *
 * @param __self - descriptor of dynastruc
 * @param __data - returned data
 * @param __tag - returned tag
 * @return zero on success, non-zero otherwise
 */
int
dyna_del_from_front (dynastruc_t *__self, void **__data, int *__tag)
{
  return dyna_pop (__self, __data, __tag);
}

/**
 * Delete element from back of list
 *
 * @param __self - descriptor of dynastruc
 * @param __data - returned data
 * @param __tag - returned tag
 * @return zero on success, non-zero otherwise
 */
int
dyna_del_from_back (dynastruc_t *__self, void **__data, int *__tag)
{
  dyna_item_t *temp_ptr;
  if (!__self)
    {
      /* Trying to work with wrong struct */
      return -1;
    }

  if (__self->count == 0)
    {
      return -1;
    }

  if (__data)
    {
      *__data = __self->tail->data;
    }

  if (__tag)
    {
      *__tag = __self->tail->tag;
    }

  temp_ptr = __self->tail;
  __self->tail = (dyna_item_t*) __self->tail->prev_ptr;
  dyna_delete (__self, temp_ptr, 0);

  if (!__self->tail)
    {
      __self->head = 0;
    }
  return 0;
}

/**
 * Check is dynastruc is empty
 *
 * @param __self - descriptor of dynastruc
 * @return non-zero if dynastruc is not empty, non-zero otherwise
 */
int
dyna_empty (dynastruc_t *__self)
{
  return __self->count == 0;
}

/**
 * Get element by index
 *
 * @param __self - descriptor of dynastruc
 * @param __i - index of element
 * @return descriptor of item
 */
dyna_item_t*
dyna_get_item_by_index (dynastruc_t *__self, int __i)
{
  int i = 0;
  dyna_item_t *cur;
  if (!__self)
    {
      return NULL;
    }
  cur = dyna_head (__self);
  while (cur && i < __i)
    {
      i++;
      cur = dyna_next (cur);
    }
  return cur;
}

/**
 * Sort dynastruc
 *
 * @param __self - descriptor of dynastruc to be sorted
 * @param __comparator - comparator of two elements
 */
void
dyna_sort (dynastruc_t *__self, dyna_comparator __comparator)
{
  int i, j, n;
  if (!__self) return;
  dyna_item_t *cur;

  n = dyna_length (__self);

  for (i = 0; i < n; i++)
    {
      cur = dyna_head (__self);
      for (j = 0; j < n - 1; j++)
        {
          if (__comparator (dyna_data (cur), dyna_data (dyna_next (cur))) > 0)
            {
              swap (cur, dyna_next (cur));
            }
          cur = dyna_next (cur);
        }
    }
}

/********
 * Default comparators
 */

/**
 * Compare strings
 *
 * @param __l, __r - strings to compare
 * @return see strcmp()
 */
int
dyna_string_comparator (void *__l, void *__r)
{
  return !strcmp (__l, __r);
}

/**
 * Check for equal
 *
 * @param __l, __r - data to compare
 * @return zero if data is different, non-zero otherwise
 */
int
dyna_eq_comparator (void *__l, void *__r)
{
  return __l == __r;
}

/**
 * Sortrer for directory listing
 *
 * @param __l, __r - names of dirents
 * @return see strcmp
 */
int
dyna_sort_listing_comparator (void *__l, void *__r)
{
  char *l = __l, *r = __r;
  if (is_integer (l) && is_integer (r) && l[0] != '-' && r[0] != '-')
    {
      long a = atol (l), b = atol (r);
      if (a < b) return -1;
      else
        if (a > b) return 1;
      else
        return 0;
    }
  return strcmp (__l, __r);
}
