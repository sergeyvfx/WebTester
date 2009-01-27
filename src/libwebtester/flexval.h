/**
 * WebTester Server - server of on-line testing system
 *
 * Flexible evalible variables.
 * Not very e2u, but funny.
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _flexval_h_
#define _flexval_h_

#include "smartinclude.h"

BEGIN_HEADER

#include <libwebtester/types.h>

#define FVT_UNDEFINED 0x00
#define FVT_INTEGER   0x01
#define FVT_STRING    0x02
#define FVT_FLOAT     0x04
#define FVT_ARRAY     0x08

#define FLEXVAL_ARRAY_LENGTH(__self)         ((__self)->array.length)
#define FLEXVAL_ARRAY_DATA(__self)           ((__self)->array.data)
#define FLEXVAL_ARRAY_ELEM(__self,__index)   ((__self)->array.data[(__index)])

typedef struct
{
  long integer;
  double real;
  char *pchar;

  __u8_t type;

  struct
  {
    __u16_t length;
    void **data;
  } array;

  __u8_t allocated;
} flex_value_t;

/********
 *
 */

/* Allocate new flexval */
flex_value_t*
flexval_alloc (void);

/* Unserialize flexval */
flex_value_t*
flexval_unserialize (const char *__data);

/* Serialize flexval */
void
flexval_serialize (flex_value_t *__self, char *__res);

/* Set value as string */
void
flexval_set_string (flex_value_t *__self, const char *__str);

/* Set value as integer */
void
flexval_set_int (flex_value_t *__self, long __value);

/* Set value as float */
void
flexval_set_float (flex_value_t *__self, double __value);

/* Set value as array */
void
flexval_set_array (flex_value_t *__self, flex_value_t **__data);

/* Get string */
char*
flexval_get_string (flex_value_t *__self);

/* Get integer */
long
flexval_get_int (flex_value_t *__self);

/* Get float */
double
flexval_get_float (flex_value_t *__self);

/* Get array */
flex_value_t**
flexval_get_array (flex_value_t *__self);

/* Get element of array */
flex_value_t*
flexval_get_array_elem (flex_value_t *__self, int __index);

/* Get integer value of array's element */
long
flexval_get_array_int (flex_value_t *__self, int __index);

/* Get float value of array's element */
double
flexval_get_array_float (flex_value_t *__self, int __index);

/* Get string of array's element */
char*
flexval_get_array_string (flex_value_t *__self, int __index);

/* Get array value of array's element */
flex_value_t**
flexval_get_array_array (flex_value_t *__self, int __index);

/* Zerolization of value */
void
flexval_zerolize (flex_value_t *__self);

/* Set the value as `undefined` */
void
flexval_set_undefined (flex_value_t *__self);

/* Duplicate flexval */
flex_value_t*
flexval_dup (flex_value_t *__self);

/* Create flexval */
void
flexval_create (flex_value_t *__self);

/* Free flexval */
void
flexval_free (flex_value_t *__self);

/* Value is truth */
int
flexval_is_truth (flex_value_t *__self);

/* Append element to array */
void
flexval_array_append (flex_value_t *__self, flex_value_t *__element);

/* Append integer value to array */
void
flexval_array_append_int (flex_value_t *__self, long __data);

/* Append float value to array */
void
flexval_array_append_float (flex_value_t *__self, double __data);

/* Append string to array */
void
flexval_array_append_string (flex_value_t *__self, const char *__data);

/* Append array to array */
void
flexval_array_append_array (flex_value_t *__self, flex_value_t **__data);

/* Smart converting string to long */
long
flexval_atol (const char *__self);

/* Smart converting string to float */
double
flexval_atolf (const char *__self);

/* Compatre two flexvals */
int
flexval_cmp (flex_value_t *__a, flex_value_t *__b);

/* Copy flexval */
void
flexval_copy (flex_value_t *__src, flex_value_t *__dst);

END_HEADER

#endif
