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

#include "flexval.h"
#include "util.h"
#include "strlib.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#define FV_ARRAY           "Array"

#define FVP_FLAGS_ERROR    0x00000001
#include "util.h"
#include "strlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#define FV_ARRAY           "Array"

#define FVP_FLAGS_ERROR    0x00000001
#define FVP_FLAGS_ARRAY    0x00000002
#define FVP_FLAGS_STRING   0x00000004

#define ERROR(text,flags) \
  { \
    strcpy (__error, text);  \
    *__flags|=flags; \
    return __data; \
  }

static void
flexval_free_array (flex_value_t *__self);

static void
flexval_create_entry (flex_value_t *__self);

/**
 * Evalue all other scalar values from string
 *
 * @param __self - descriptor of flexval
 */
static void
flexval_eval_from_string (flex_value_t *__self)
{
  if (!__self) return;
  if (!is_number (__self->pchar))
    {
      __self->integer = __self->real = flexval_is_truth (__self);
      return;
    }
  __self->integer = flexval_atol (__self->pchar);
  __self->real = flexval_atolf (__self->pchar);
  __self->type = FVT_STRING;
}

/**
 * Evalue all other scalar values from integer
 *
 * @param __self - descriptor of flexval
 */
static void
flexval_eval_from_int (flex_value_t *__self)
{
  char flexval_buffer[65536];
  if (!__self) return;
  SAFE_FREE (__self->pchar);
  __self->real = (double) __self->integer;
  sprintf (flexval_buffer, "%ld", __self->integer);
  __self->pchar = malloc (strlen (flexval_buffer) + 1);
  strcpy (__self->pchar, flexval_buffer);
  __self->type = FVT_INTEGER;
}

/**
 * Evalue all other scalar values from float
 *
 * @param __self - descriptor of flexval
 */
static void
flexval_eval_from_float (flex_value_t *__self)
{
  char flexval_buffer[65536];
  if (!__self) return;
  SAFE_FREE (__self->pchar);
  __self->integer = (long) __self->real;

  /* Big troules: need to write round() */
  sprintf (flexval_buffer, "%.12lf", __self->real);

  drop_triling_zeroes (flexval_buffer);
  __self->pchar = malloc (strlen (flexval_buffer) + 1);
  strcpy (__self->pchar, flexval_buffer);
  __self->type = FVT_FLOAT;
}

/****
 *
 */

/**
 * Unserialization parser iterator
 *
 * @param __data - data to parse
 * @param __token - found token
 * @param __linenum - number of line
 * @param __flags - different flags of token
 * @param __error - error description
 * @return new shift of data
 */
static char*
unserialize_iterator (char *__data, char *__token, int *__linenum,
                      int *__flags, char *__error)
{
  char ch;
  int len = 0;

  strcpy (__error, "");
  *__flags = 0;
  *__token = 0;

  if (!__data || !*__data)
    {
      return 0;
    }

  ch = *__data;
  while (ch <= 32 || ch >= 127)
    {
      if (!ch)
        {
          return 0;
        }

      if (ch == '\n' && __linenum)
        {
          (*__linenum)++;
        }

      ch = *(++__data);
    }

  if (ch == '[')  /* Array */
    {
      int interior = 1;
      ch = *(++__data);
      while (interior > 0)
        {
          if (!ch) ERROR ("Unexpected end of file", FVP_FLAGS_ERROR);
          if (ch == '\n' && __linenum)
            {
              (*__linenum)++;
            }

          if (ch == '[')
            {
              interior++;
            }

          if (ch == ']')
            {
              interior--;
            }

          *(__token + len++) = ch;
          ch = *(++__data);
        }
      *(__token + len - 1) = 0;
      *__flags |= FVP_FLAGS_ARRAY;
      return __data;
    }
  else
    if (ch == '"') /* String */
    {
      ch = *(++__data);

      while (ch != '"')
        {
          if (ch == '"')
            {
              break;
            }

          if (!ch)
            {
              ERROR ("Unexpected end of file", FVP_FLAGS_ERROR);
            }

          if (ch == '\n')
            {
              ERROR ("Unexpected end of line", FVP_FLAGS_ERROR);
            }

          if (ch == '\\')
            {
              ch = *(++__data);
              if (!ch) ERROR ("Unexpected end of file", FVP_FLAGS_ERROR);
              if (ch == '\n') ERROR ("Unexpected end of line", FVP_FLAGS_ERROR);
              if (ch == 't') *(__token + len++) = '\t'; else
              if (ch == 'r') *(__token + len++) = '\r'; else
              if (ch == 'n') *(__token + len++) = '\n'; else
                *(__token + len++) = ch;
            }
          else
            {
              *(__token + len++) = ch;
            }
          ch = *(++__data);
        }
      *__flags |= FVP_FLAGS_STRING;
      *(__token + len) = 0;
      return ++__data;
    }
  else
    {
      while (ch > 32 && ch < 127)
        {
          *(__token + len++) = ch;
          ch = *(++__data);
        }
      *(__token + len) = 0;
      return __data;
    }
}

/**
 * Unserialization of flexvalue
 *
 * @param __data - serialized string
 * @param __base_line - base line number
 * @param __error - error description
 * @return descriptor of new flexval
 */
static flex_value_t*
flexval_unserialize_entry (const char *__data, int __base_line, char *__error)
{
  char error[65536];
  int linenum = __base_line, prevline = 0;
  int flags;
  char *shift = (char*)__data;
  char token[65536], dummy[65536];
  flex_value_t *ptr = 0;

  while ((shift = unserialize_iterator (shift, token, &linenum,
                                        &flags, error)))
    {
      if (flags & FVP_FLAGS_ERROR)
        {
          goto __fail_;
        }

      if (flags & FVP_FLAGS_ARRAY)
        {
          char *newdata = malloc (65536);
          char *shift;

          strcpy (newdata, token);
          shift = newdata;

          /* Allocate new value */
          ptr = flexval_alloc ();
          flexval_set_string (ptr, FV_ARRAY);
          ptr->type = FVT_ARRAY;
          prevline = linenum;
          while ((shift = unserialize_iterator (shift, token, &linenum,
                                                &flags, error)))
            {
              if (flags & FVP_FLAGS_ERROR)
                {
                  free (newdata);
                  goto __fail_;
                }
              if (flags & FVP_FLAGS_ARRAY)
                {
                  flex_value_t *tmp;
                  sprintf (dummy, "[%s]", token);
                  tmp = flexval_unserialize_entry (dummy, prevline, error);
                  if (strcmp (error, ""))
                    {
                      if (tmp) flexval_free (tmp);
                      free (newdata);
                      goto __fail_;
                    }
                  flexval_array_append (ptr, tmp);
                  flexval_free (tmp);
                }
              else
                {
                  if (flags & FVP_FLAGS_STRING)
                    flexval_array_append_string (ptr, token);
                  else
                    {
                      if (is_integer (token))
                        {
                          flexval_array_append_int (ptr, flexval_atol (token));
                        }
                      else
                        {
                          if (is_number (token))
                            {
                              flexval_array_append_float (ptr,
                                                        flexval_atolf (token));
                            }
                        else
                          {
                            flexval_array_append_string (ptr, token);
                          }
                        }
                    }
                }
              prevline = linenum;
            }

          free (newdata);
        }
      else
        {
          trim (shift, dummy);
          /* Check for da extra information */
          if (strcmp (dummy, ""))
            {
              strcpy (error, "Extra information");
              goto __fail_;
            }

          /* Allocate new value */
          ptr = flexval_alloc ();

          /* Smart setting of value */
          if (flags & FVP_FLAGS_STRING)
            {
              flexval_set_string (ptr, token);
            }
          else
            {
              if (is_integer (token))
                {
                  flexval_set_int (ptr, flexval_atol (token));
                }
              else
                {
                  if (is_number (token))
                    {
                      flexval_set_float (ptr, flexval_atolf (token));
                    }
                  else
                    {
                      flexval_set_string (ptr, token);
                    }
                }
            }
        }
      prevline = linenum;
    }

  if (flags & FVP_FLAGS_ERROR) goto __fail_;

  return ptr;

__fail_:
  if (__error) sprintf (__error, "%s at line %d", error, linenum);
  return ptr;
}

/**
 * Serializator for flexval
 *
 * @param __self - descriptor of flexval to serialize
 * @param __res - serialized string
 */
static void
flexval_serialize_entry (flex_value_t *__self, char *__res)
{
  if (!__self)
    {
      return;
    }

  if (__self->type == FVT_STRING || __self->type == FVT_UNDEFINED)
    {
      strcat (__res, "\"");
      strcat (__res, flexval_get_string (__self));
      strcat (__res, "\"");
    }
  else
    if (__self->type == FVT_INTEGER)
    {
      char dummy[1024];
      sprintf (dummy, "%ld", flexval_get_int (__self));
      strcat (__res, dummy);
    }
  else
    if (__self->type == FVT_FLOAT)
    {
      char dummy[1024];
      sprintf (dummy, "%lf", flexval_get_float (__self));
      strcat (__res, dummy);
    }
  else
    if (__self->type == FVT_ARRAY)
    {
      int i, n;
      strcat (__res, "[");
      for (i = 0, n = FLEXVAL_ARRAY_LENGTH (__self); i < n; i++)
        {
          flexval_serialize_entry (FLEXVAL_ARRAY_ELEM (__self, i), __res);
          if (i != n - 1)
            {
              strcat (__res, " ");
            }
        }
      strcat (__res, "]");
    }
}

/**
 * Fill new descriptor
 *
 * @param __self - descriptor to manipulate with
 */
static void
flexval_create_entry (flex_value_t *__self)
{
  __self->pchar = 0;
  FLEXVAL_ARRAY_LENGTH (__self) = 0;
  FLEXVAL_ARRAY_DATA (__self) = 0;
  flexval_zerolize (__self);
  flexval_set_undefined (__self);
}

/**
 * Entry point for array deleter
 *
 * @param __self - descriptor of flexval to free
 * @param __interior - level of ineriority
 */
static void
flexval_free_array_entry (flex_value_t *__self, int __interior)
{
  int i;
  if (!__self)
    {
      return;
    }

  if (__interior)
    {
      SAFE_FREE (__self->pchar);
      if (__self->allocated) free (__self);
    }

  for (i = 0; i < FLEXVAL_ARRAY_LENGTH (__self); i++)
    {
      flexval_free_array_entry (FLEXVAL_ARRAY_ELEM (__self, i),
                                __interior + 1);
    }

  SAFE_FREE (FLEXVAL_ARRAY_DATA (__self));
  FLEXVAL_ARRAY_DATA (__self) = 0;
}

/**
 * Free array
 *
 * @param __self - descriptor of flexval
 */
static void
flexval_free_array (flex_value_t *__self)
{
  flexval_free_array_entry (__self, 0);
}

/********
 * User's backend
 */

/**
 * Allocate new flexval
 *
 * @return allocated descriptor
 */
flex_value_t*
flexval_alloc (void)
{
  flex_value_t *ptr = malloc (sizeof (flex_value_t));
  ptr->allocated = 1;
  flexval_create_entry (ptr);
  return ptr;
}

/**
 * Unserialize flexval
 *
 * @param __data - string contains serialized data
 * @return descriptor of new flexval
 */
flex_value_t*
flexval_unserialize (const char *__data)
{
  return flexval_unserialize_entry (__data, 0, 0);
}

/**
 * Serialize flexval
 *
 * @param __self - flexval to serialize
 * @param __res - serialized data
 */
void
flexval_serialize (flex_value_t *__self, char *__res)
{
  strcpy (__res, "");
  flexval_serialize_entry (__self, __res);
}

/**
 * Set value as string
 *
 * @param __self - descriptor of flexval
 * @param __str - new value
 */
void
flexval_set_string (flex_value_t *__self, const char *__str)
{
  char flexval_buffer[65536];
  if (!__self || !__str)
    {
      return;
    }
  SAFE_FREE (__self->pchar);
  snprintf (flexval_buffer, 65535, "%s", __str);
  __self->pchar = malloc (strlen (flexval_buffer) + 1);
  flexval_free_array (__self);
  strcpy (__self->pchar, flexval_buffer);
  flexval_eval_from_string (__self);
}

/**
 * Set value as integer
 *
 * @param __self - descriptor of flexval
 * @param __str - new value
 */
void
flexval_set_int (flex_value_t *__self, long __value)
{
  if (!__self)
    {
      return;
    }
  flexval_free_array (__self);
  __self->integer = __value;
  flexval_eval_from_int (__self);
}

/**
 * Set value as float
 *
 * @param __self - descriptor of flexval
 * @param __str - new value
 */
void
flexval_set_float (flex_value_t *__self, double __value)
{
  if (!__self)
    {
      return;
    }
  flexval_free_array (__self);
  __self->real = __value;
  flexval_eval_from_float (__self);
}

/**
 * Duplicate flexval
 *
 * @param __self - source flexval
 * @preturn descriptor of clone
 */
flex_value_t*
flexval_dup (flex_value_t *__self)
{
  flex_value_t *ptr;

  if (!__self)
    {
      return 0;
    }

  ptr = flexval_alloc ();

  /* Default dumping */
  flexval_set_string (ptr, __self->pchar);
  ptr->integer = __self->integer;
  ptr->real = __self->real;

  FLEXVAL_ARRAY_LENGTH (ptr) = 0;
  FLEXVAL_ARRAY_DATA (ptr) = 0;

  ptr->type = __self->type;

  if (__self->type == FVT_ARRAY) /* Dumping of array */
    {
      int i, n;
      n = FLEXVAL_ARRAY_LENGTH (__self);

      FLEXVAL_ARRAY_LENGTH (ptr) = n;
      FLEXVAL_ARRAY_DATA (ptr) = malloc (sizeof (flex_value_t*)*(n + 1));

      for (i = 0; i < n; i++)
        {
          /* Duplicatimg of eacj element in array */
          FLEXVAL_ARRAY_ELEM (ptr, i) =
                  flexval_dup (FLEXVAL_ARRAY_ELEM (__self, i));
        }

      FLEXVAL_ARRAY_ELEM (ptr, n) = 0;
    }

  return ptr;
}

/**
 * Set value as array
 *
 * @param __self - descriptor of flexval
 * @param __data - array of values
 */
void
flexval_set_array (flex_value_t *__self, flex_value_t **__data)
{
  int i, n = 0;
  flexval_zerolize (__self);

  strcpy (__self->pchar, FV_ARRAY);

  /* Calculate length of assignment */
  while (__data[n])
    {
      n++;
    }

  /* Alloc memory and store length */
  FLEXVAL_ARRAY_DATA (__self) = malloc (sizeof (flex_value_t*)*(n + 1));
  FLEXVAL_ARRAY_LENGTH (__self) = n;

  /* Duplicate data */
  for (i = 0; i < n; i++)
    {
      /* Duplicatin each element from assignment array */
      FLEXVAL_ARRAY_ELEM (__self, i) = flexval_dup (__data[i]);
    }

  FLEXVAL_ARRAY_ELEM (__self, n) = 0;

  __self->type = FVT_ARRAY;
}

/**
 * Deprecated?
 */
void
flexval_set_serialized_array_entry (flex_value_t *__self, char *__data,
                                    int __base_line, char *__error)
{
  flex_value_t *ptr = 0;
  ptr = flexval_unserialize_entry (__data, __base_line, __error);

  FLEXVAL_ARRAY_DATA (__self) = FLEXVAL_ARRAY_DATA (ptr);
  FLEXVAL_ARRAY_LENGTH (__self) = FLEXVAL_ARRAY_LENGTH (ptr);

  flexval_set_array (__self, (flex_value_t**) FLEXVAL_ARRAY_DATA (ptr));

  SAFE_FREE (ptr->pchar);
}

/**
 * Get string
 *
 * @param __self - descriptor of flexval
 * @return string stored in value
 */
char*
flexval_get_string (flex_value_t *__self)
{
  if (__self)
    {
      return __self->pchar;
    }
  return "";
}

/**
 * Get integer
 *
 * @param __self - descriptor of flexval
 * @return integer value stored in value
 */
long
flexval_get_int (flex_value_t *__self)
{
  if (__self)
    {
      return __self->integer;
    }
  return 0;
}

/**
 * Get float
 *
 * @param __self - descriptor of flexval
 * @return float value stored in value
 */
double
flexval_get_float (flex_value_t *__self)
{
  if (__self)
    {
      return __self->real;
    }
  return 0;
}

/**
 * Get array
 *
 * @param __self - descriptor of flexval
 * @return array stored in value
 */
flex_value_t**
flexval_get_array (flex_value_t *__self)
{
  if (__self)
    {
      return (flex_value_t**) FLEXVAL_ARRAY_DATA (__self);
    }
  return 0;
}

/**
 * Get element of array
 *
 * @param __self - descriptor of flexval
 * @param __index - index of element to get
 * @return element of array
 */
flex_value_t*
flexval_get_array_elem (flex_value_t *__self, int __index)
{
  if (__self && __index >= 0 && __index < FLEXVAL_ARRAY_LENGTH (__self))
    {
      return (flex_value_t*) FLEXVAL_ARRAY_ELEM (__self, __index);
    }
  return 0;
}

/**
 * Get integer value of array's element
 *
 * @param __self - descriptor of flexval
 * @param __index - index of element
 * @return integer value stored in array's element
 */
long
flexval_get_array_int (flex_value_t *__self, int __index)
{
  return flexval_get_int (flexval_get_array_elem (__self, __index));
}

/**
 * Get float value of array's element
 *
 * @param __self - descriptor of flexval
 * @param __index - index of element
 * @return float value stored in array's element
 */
double
flexval_get_array_float (flex_value_t *__self, int __index)
{
  return flexval_get_float (flexval_get_array_elem (__self, __index));
}

/**
 * Get string of array's element
 *
 * @param __self - descriptor of flexval
 * @param __index - index of element
 * @return string stored in array's element
 */
char*
flexval_get_array_string (flex_value_t *__self, int __index)
{
  return flexval_get_string (flexval_get_array_elem (__self, __index));
}

/**
 * Get array value of array's element
 *
 * @param __self - descriptor of flexval
 * @param __index - index of element
 * @return array stored in array's element
 */
flex_value_t**
flexval_get_array_array (flex_value_t *__self, int __index)
{
  return flexval_get_array (flexval_get_array_elem (__self, __index));
}

/**
 * Zerolization of value
 *
 * @param  __self - descriptor of flexval to be zerolized
 */
void
flexval_zerolize (flex_value_t *__self)
{
  flexval_set_string (__self, "");
}

/**
 * Set the value as `undefined`
 *
 * @param __self - descriptor of flexval to manipulate with
 */
void
flexval_set_undefined (flex_value_t *__self)
{
  __self->type = FVT_UNDEFINED;
}

/**
 * Create flexval
 *
 * @param __self - descriptor of flexval
 */
void
flexval_create (flex_value_t *__self)
{
  __self->allocated = 0;
  flexval_create_entry (__self);
}

/**
 * Free flexval
 *
 * @param _-flexval - flexval to free
 */
void
flexval_free (flex_value_t *__self)
{
  if (!__self)
    {
      return;
    }
  SAFE_FREE (__self->pchar);
  flexval_free_array (__self);
  if (__self->allocated) free (__self);
}

/**
 * Value is truth
 *
 * @return non-zero if flexval is trooth, zero otherwise
 */
int
flexval_is_truth (flex_value_t *__self)
{
  return is_truth (flexval_get_string (__self));
}

/********
 * FlexVal ARRAY stuff
 */

/**
 * Append element to array
 *
 * @param __self - descriptor of fexval
 * @param __element - element to append
 */
void
flexval_array_append (flex_value_t *__self, flex_value_t *__element)
{
  int i, n;

  flex_value_t *ptr;
  flex_value_t **sdata;
  if (!__self || !__element)
    {
      return;
    }

  __self->integer = __self->real = 0;
  strcpy (__self->pchar, FV_ARRAY);
  __self->type = FVT_ARRAY;

  ptr = flexval_dup (__element);

  n = FLEXVAL_ARRAY_LENGTH (__self);

  sdata = (flex_value_t**) FLEXVAL_ARRAY_DATA (__self);
  FLEXVAL_ARRAY_DATA (__self) = malloc (sizeof (flex_value_t*)*(n + 2));

  for (i = 0; i < n; i++)
    {
      FLEXVAL_ARRAY_ELEM (__self, i) = sdata[i];
    }

  FLEXVAL_ARRAY_ELEM (__self, n) = ptr;
  FLEXVAL_ARRAY_ELEM (__self, n + 1) = 0;

  FLEXVAL_ARRAY_LENGTH (__self)++;

  SAFE_FREE (sdata);
}

/**
 * Append integer value to array
 *
 * @param __self - descriptor of flexval
 * @param __data - value to append
 */
void
flexval_array_append_int (flex_value_t *__self, long __data)
{
  flex_value_t data;
  flexval_create (&data);
  flexval_set_int (&data, __data);
  flexval_array_append (__self, &data);
  flexval_free (&data);
}

/**
 * Append float value to array
 *
 * @param __self - descriptor of flexval
 * @param __data - value to append
 */
void
flexval_array_append_float (flex_value_t *__self, double __data)
{
  flex_value_t data;
  flexval_create (&data);
  flexval_set_float (&data, __data);
  flexval_array_append (__self, &data);
  flexval_free (&data);
}

/**
 * Append string to array
 *
 * @param __self - descriptor of flexval
 * @param __data - value to append
 */
void
flexval_array_append_string (flex_value_t *__self, const char *__data)
{
  flex_value_t data;
  flexval_create (&data);
  flexval_set_string (&data, __data);
  flexval_array_append (__self, &data);
  flexval_free (&data);
}

/**
 * Append array to array
 *
 * @param __self - descriptor of flexval
 * @param __data - value to append
 */
void
flexval_array_append_array (flex_value_t *__self, flex_value_t **__data)
{
  flex_value_t data;
  flexval_create (&data);
  flexval_set_array (&data, __data);
  flexval_array_append (__self, &data);
  flexval_free (&data);
}

/**
 * Compatre two flexvals
 *
 * @param __a, __b - flexvals to be compared
 * @return the same as strcmp()
 */
int
flexval_cmp (flex_value_t *__a, flex_value_t *__b)
{
  char a[65536], b[65536];

  if (__a->type != __b->type)
    {
      return -2;
    }

  if (__a->type == FVT_UNDEFINED)
    {
      return 0;
    }

  if (__a->type != FVT_ARRAY)
    {
      return strcmp (__a->pchar, __b->pchar);
    }

  /* FIXME: Big troubles! */
  flexval_serialize (__a, a);
  flexval_serialize (__b, b);

  return strcmp (a, b);
}

/**
 * Copy flexval
 *
 * @param __src - source flexval
 * @param __dst - destination flexval
 */
void
flexval_copy (flex_value_t *__src, flex_value_t *__dst)
{
  if (!__src || !__dst)
    {
      return;
    }

  if (__src->type == FVT_UNDEFINED)
    {
      return;
    }

  if (__src->type == FVT_INTEGER)
    flexval_set_int (__dst, __src->integer);
  else
    if (__src->type == FVT_FLOAT)
    flexval_set_float (__dst, __src->real);
  else
    if (__src->type == FVT_STRING)
    flexval_set_string (__dst, __src->pchar);
  else
    if (__src->type == FVT_ARRAY)
    {
      /*
       * TODO: Do not forget to write this stuff!!
       */
    }
}

/********
 *
 */

/**
 * Smart converting string to long
 *
 * @param __self - string to convert
 * @return converted value
 */
long
flexval_atol (const char *__self)
{
  /*
   * TODO: Maybe move this stuff onto utils.c?
   */

  int sign;
  long val;
  if (!__self) return 0;

  if (*__self == '-')
    {
      sign = -1;
      __self++;
    }
  else sign = 1;

  val = 0;

  /* check for HEX */
  if (*__self == '0' && (__self[1] == 'x' || __self[1] == 'X'))
    {
      __self += 2;
      for (;;)
        {
          if (*__self >= '0' && *__self <= '9')
            val = (val << 4) + *__self - '0';
          else
            if (*__self >= 'a' && *__self <= 'f')
              val = (val << 4) + *__self - 'a' + 10;
            else
              if (*__self >= 'A' && *__self <= 'F')
                val = (val << 4) + *__self - 'A' + 10;
              else
                return sign*val;
          __self++;
        }
    }

  /* check for character */
  if (*__self == '\'')
    return sign * __self[1];

  for (;;)
    {
      if (*__self < '0' || *__self > '9')
        {
          return sign * val;
        }
      val = val * 10 + *__self - '0';
      __self++;
    }
}

/**
 * Smart converting string to float
 *
 * @param __self - string to convert
 * @return converted value
 */
double
flexval_atolf (const char *__self)
{

  /*
   * TODO: Maybe move this stuff onto utils.c?
   */

  char *str = (char*)__self;
  int sign = 0;
  double val;
  long decimal, total;

  if (!str)
    {
      return 0.0;
    }

  val = 0;
  if (*str == '-')
    {
      sign = -1;
      str++;
    }
  else
    {
      sign = 1;
    }

  /* check for HEX */
  if (*str == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
      *str += 2;
      for (;;)
        {
          if (*str >= '0' && *str <= '9')
            {
              val = val * 16 + *str - '0';
            }
          else if (*str >= 'a' && *str <= 'f')
            {
              val = val * 16 + *str - 'a' + 10;
            }
          else if (*str >= 'A' && *str <= 'F')
            {
              val = val * 16 + *str - 'A' + 10;
            }
          else
            {
              return sign*val;
            }
          str++;
        }
    }

  /* check for character */
  if (*str == '\'')
    {
      return sign * str[1];
    }
  decimal = -1;
  total = 0;
  for (;;)
    {
      if (*str == '.')
        {
          decimal = total;
          str++;
          continue;
        }
      if (*str < '0' || *str > '9')
        {
          break;
        }
      val = val * 10 + *str - '0';
      str++;
      total++;
    }
  if (decimal == -1)
    {
      return sign * val;
    }
  while (total > decimal)
    {
      val /= 10;
      total--;
    }

  return sign*val;
}
