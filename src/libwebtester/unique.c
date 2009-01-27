/**
 * WebTester Server - server of on-line testing system
 *
 * Uique number generator
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "unique.h"

#include <malloc.h>
#include <memory.h>

/**
 * Create pool of unique numbers
 *
 * @return new pool
 */
unique_pool_t*
unique_pool_create (void)
{
  long i;
  unique_pool_t *ptr;
  ptr = malloc (sizeof (unique_pool_t));

  for (i = 0; i < MAX_UNIQUE; i++)
    {
      ptr->stack[i] = i;
    }

  ptr->ptr = 0;

  return ptr;
}

/**
 * Destroy pool of unique numbers
 *
 * @param __self - pool to destroy
 */
void
unique_pool_destroy (unique_pool_t *__self)
{
  if (!__self)
    {
      return;
    }
  free (__self);
}

/**
 * Allocate new unique id
 *
 * @param __self - pool with unique ids
 * @return new unique id
 */
int
unique_alloc_uid (unique_pool_t *__self)
{
  if (!__self || __self->ptr >= MAX_UNIQUE)
    {
      return -1;
    }
  return __self->stack[__self->ptr++];
}

/**
 * Release allocated unique id
 *
 * @param __self - pool of ids
 * @param __uid - id to release
 */
void
unique_release_uid (unique_pool_t *__self, unsigned short __uid)
{
  if (!__self || __self->ptr <= 0)
    {
      return;
    }
  __self->stack[--__self->ptr] = __uid;
}

/**
 * Check if pool is empty
 *
 * @param __self - pool to check
 * @return non-zero if pool is empty, zero otherwise
 */
BOOL
unique_pool_empty (unique_pool_t *__self)
{
  if (!__self)
    {
      return -1;
    }
  return __self->ptr >= MAX_UNIQUE;
}

/**
 * Check if pool is full
 *
 * @param __self - pool to check
 * @return non-zero if pool is full, zero otherwise
 */
BOOL
unique_pool_full (unique_pool_t *__self)
{
  if (!__self) return -1;
  return __self->ptr == 0;
}
