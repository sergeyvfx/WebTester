/*
 * ================================================================================
 *  unique.h - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _unique_h_
#define _unique_h_

#include <libwebtester/smartinclude.h>

#define MAX_UNIQUE 65536

typedef struct {
  unsigned short stack[MAX_UNIQUE];
  long ptr;
} unique_pool_t;

void
unique_done                        (void);

unique_pool_t*
unique_pool_create                 (void);

void
unique_pool_destroy                (unique_pool_t *__self);

int
unique_alloc_uid                   (unique_pool_t *__self);

void
unique_release_uid                 (unique_pool_t *__self, unsigned short __uid);

BOOL
unique_pool_empty                  (unique_pool_t *__self);

BOOL
unique_pool_full                   (unique_pool_t *__self);

#endif
