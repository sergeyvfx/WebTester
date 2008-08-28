/*
 *
 * ================================================================================
 *  flexVal.h
 * ================================================================================
 *
 *  Flexible evalutable variables.
 *  Not very e2u, but funny.
 *  
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _flexval_h_
#define _flexval_h_

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
  long     integer;
  double   real;
  char     *pchar;

  __u8_t   type;

  struct {
    __u16_t length;
    void    **data;
  } array;

  //
  __u8_t allocated;
} flex_value_t;

flex_value_t*
flexval_alloc                      (void);

flex_value_t*
flexval_unserialize                (char *__data);

void
flexval_serialize                  (flex_value_t *__self, char *__res);

void
flexval_set_string                 (flex_value_t *__self, char *__str);

void
flexval_set_int                    (flex_value_t *__self, long __value);

void
flexval_set_float                  (flex_value_t *__self, double __value);

void
flexval_set_array                  (flex_value_t *__self, flex_value_t **__data);

void
flexval_set_serialized_array_entry (flex_value_t *__self, char *__data, int __base_line, char *__error);

char*
flexval_get_string                 (flex_value_t *__self);

long
flexval_get_int                    (flex_value_t *__self);

double
flexval_get_float                  (flex_value_t *__self);

flex_value_t**
flexval_get_array                  (flex_value_t *__self);

flex_value_t*
flexval_get_array_elem             (flex_value_t *__self, int __index);

long
flexval_get_array_int              (flex_value_t *__self, int __index);

double
flexval_get_array_float            (flex_value_t *__self, int __index);

char*
flexval_get_array_string           (flex_value_t *__self, int __index);

flex_value_t**
flexval_get_array_array            (flex_value_t *__self, int __index);

void
flexval_zerolize                   (flex_value_t *__self);

void
flexval_set_undefined              (flex_value_t *__self);

flex_value_t*
flexval_dup                        (flex_value_t *__self);

void
flexval_create                     (flex_value_t *__self);

void
flexval_free                       (flex_value_t *__self);

int
flexval_is_truth                   (flex_value_t *__self);

void
flexval_array_append               (flex_value_t *__self, flex_value_t *__element);

void
flexval_array_append_int           (flex_value_t *__self, long __data);

void
flexval_array_append_float         (flex_value_t *__self, double __data);

void
flexval_array_append_string        (flex_value_t *__self, char *__data);

void
flexval_array_append_array         (flex_value_t *__self, flex_value_t **__data);

long
flexval_atol                       (char *__self);

double
flexval_atolf                      (char *__self);

int
flexval_cmp                        (flex_value_t *__a, flex_value_t *__b);

void
flexval_copy                       (flex_value_t *__src, flex_value_t *__dst);

#endif
