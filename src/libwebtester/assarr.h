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

#include "smartinclude.h"

#ifndef _assarr_h_
#define _assarr_h_

BEGIN_HEADER

/********
 * Type defenitions
 */

typedef struct
{
  char *key;
  void *value;
  void *next_ptr;
} assarr_entry_t;

typedef struct
{
  long m;
  long k;
  int count;
  assarr_entry_t **data;
} assarr_t;

typedef void (*assarr_deleter) (void *__item);

/********
 * Macroses
 */

#define ASSARR_FOREACH_DO(__self, __key, __value) \
 { \
  int __i_; \
  assarr_entry_t *__cur_, *__next_; \
  for (__i_=0; __i_ < (__self)->m; __i_++) \
    { \
      __cur_=(__self)->data[__i_]; \
      while (__cur_) \
        { \
          __key=__cur_->key; \
          __value=__cur_->value; \
          __next_=__cur_->next_ptr; \
          {

#define ASSARR_FOREACH_DONE \
          } \
          __cur_=__next_; \
        } \
    } \
 }

/* Deleter of element of assaciative array */
/* Calls free() for each element */
void
assarr_deleter_free_ref_data (void *__item);

/* Create new assaciative array */
assarr_t*
assarr_create (void);

/* Destroy assaciative array */
void
assarr_destroy (assarr_t *__self, assarr_deleter __deleter);

/* Get value by key */
void*
assarr_get_value (assarr_t *__self, const char *__key);

/* Check if element is set */
int
assarr_isset (assarr_t *__self, const char *__key);

/* Set value by key */
void
assarr_set_value (assarr_t *__self, const char *__key, void *__value);

/* Unset value by key */
int
assarr_unset_value (assarr_t *__self, const char *__key,
                    assarr_deleter __deleter);

/* Unset all values */
int
assarr_unset_all (assarr_t *__self, assarr_deleter __deleter);

/* Pack ass. array PCHAR data to string */
/* !!! WARNING !!! Works ONLY with PCHAR data */
void
assarr_pack (assarr_t *__self, char **__out);

/* Unpack string to assaciative array */
void
assarr_unpack (const char *__data, assarr_t *__arr);

/* Get count of elements in array */
int
assarr_get_count (assarr_t *__self);

END_HEADER

#endif
