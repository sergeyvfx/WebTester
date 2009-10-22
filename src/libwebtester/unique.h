/**
 * WebTester Server - server of on-line testing system
 *
 * Uique number generator
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _unique_h_
#define _unique_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

/********
 * Constants
 */

#define MAX_UNIQUE 65536

/********
 * Type definitions
 */

typedef struct
{
  unsigned short stack[MAX_UNIQUE];
  long ptr;
} unique_pool_t;

/********
 *
 */

/* Create pool of unique numbers */
unique_pool_t*
unique_pool_create (void);

/* Destroy pool of unique numbers */
void
unique_pool_destroy (unique_pool_t *__self);

/* Allocate new unique id */
int
unique_alloc_uid (unique_pool_t *__self);

/* Release allocated unique id */
void
unique_release_uid (unique_pool_t *__self, unsigned short __uid);

/* Check if pool is empty */
BOOL
unique_pool_empty (unique_pool_t *__self);

/* Check if pool is full */
BOOL
unique_pool_full (unique_pool_t *__self);

END_HEADER

#endif
