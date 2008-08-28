/*
 *
 * ================================================================================
 *  flexval.c
 * ================================================================================
 *
 *  Flexible evalible variables.
 *  Not very e2u, but funny.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "flexval.h"
#include "smartinclude.h"
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
flexval_free_array                 (flex_value_t *__self);

static void
flexval_create_entry               (flex_value_t *__self);

flex_value_t*
flexval_alloc                      (void)
{
  flex_value_t *ptr=malloc (sizeof (flex_value_t));
  ptr->allocated=1;
  flexval_create_entry (ptr);
  return ptr;
}

static void     // Evalue all other scalar values from string
flexval_eval_from_string           (flex_value_t *__self)
{
  if (!__self) return;
  if (!is_number (__self->pchar))
    {
      __self->integer=__self->real=__self->real=flexval_is_truth (__self);
      return;
    }
  __self->integer = flexval_atol (__self->pchar);
  __self->real = flexval_atolf (__self->pchar);
  __self->type=FVT_STRING;
}

static void     // Evalue all other scalar values from integer
flexval_eval_from_int              (flex_value_t *__self)
{
  char flexval_buffer[65536];
  if (!__self) return;
  SAFE_FREE (__self->pchar);
  __self->real = (double)__self->integer;
  sprintf (flexval_buffer, "%ld", __self->integer);
  __self->pchar = malloc (strlen (flexval_buffer)+1);
  strcpy (__self->pchar, flexval_buffer);
  __self->type=FVT_INTEGER;
}

static void     // Evalue all other scalar values from float
flexval_eval_from_float            (flex_value_t *__self)
{
  char flexval_buffer[65536];
  if (!__self) return;
  SAFE_FREE (__self->pchar);
  __self->integer = (long)__self->real;
  // Big troules: need to write round()
  sprintf (flexval_buffer, "%.12lf", __self->real);
  drop_triling_zeroes (flexval_buffer);
  __self->pchar = malloc (strlen (flexval_buffer)+1);
  strcpy (__self->pchar, flexval_buffer);
  __self->type=FVT_FLOAT;
}

////////////////
//

static char*    // Unserialization parser iterator
unserialize_iterator               (char *__data, char *__token, int *__linenum, int *__flags, char *__error)
{
  char ch;
  int len=0;
  
  strcpy (__error, "");
  *__flags=0;
  *__token=0;

  if (!__data || !*__data) return 0;

//__skip_while_:
  ch=*__data;
  while (ch<=32 || ch>=127)
    {
      if (!ch) return 0;
      if (ch=='\n' && __linenum) (*__linenum)++;
      ch=*(++__data);
    }

  if (ch=='[')  // Array
    {
      int interior=1;
      ch=*(++__data);
      while (interior>0)
        {
          if (!ch) ERROR ("Unexpected end of file", FVP_FLAGS_ERROR);
          if (ch=='\n' && __linenum) (*__linenum)++;
          if (ch=='[') interior++;
          if (ch==']') interior--;
          *(__token+len++)=ch;
          ch=*(++__data);
        }
        *(__token+len-1)=0;
        *__flags|=FVP_FLAGS_ARRAY;
        return __data;
    } else
  if (ch=='"')  // String
    {
      ch=*(++__data);

      while (ch!='"')
        {
          if (ch=='"')  break;
          if (!ch)      ERROR ("Unexpected end of file", FVP_FLAGS_ERROR);
          if (ch=='\n') ERROR ("Unexpected end of line", FVP_FLAGS_ERROR);
          if (ch=='\\')
            {
              ch=*(++__data);
              if (!ch)      ERROR ("Unexpected end of file", FVP_FLAGS_ERROR);
              if (ch=='\n') ERROR ("Unexpected end of line", FVP_FLAGS_ERROR);
              if (ch=='t') *(__token+len++)='\t'; else
              if (ch=='r') *(__token+len++)='\r'; else
              if (ch=='n') *(__token+len++)='\n'; else
                *(__token+len++)=ch;
            } else
              *(__token+len++)=ch;
          ch=*(++__data);
        }
      *__flags|=FVP_FLAGS_STRING;
      *(__token+len)=0;
      return ++__data;
    } else {
      while (ch>32 && ch<127)
        {
          *(__token+len++)=ch;
          ch=*(++__data);
        }
      *(__token+len)=0;
      return __data;
    }
}

flex_value_t*   // Unserialization of flexvalue
flexval_unserialize_entry          (char *__data, int __base_line, char *__error)
{
  char error[65536];
  int linenum=__base_line, prevline=0;
  int flags;
  char *shift=__data;
  char token[65536], dummy[65536];
  flex_value_t *ptr=0;

  while ((shift=unserialize_iterator (shift, token, &linenum, &flags, error)))
    {
      if (flags&FVP_FLAGS_ERROR) goto __fail_;
 
      if (flags&FVP_FLAGS_ARRAY)
        {
          char *newdata=malloc (65536);
          char *shift;

          strcpy (newdata, token);
          shift=newdata;

          // Allocate new value
          ptr=flexval_alloc ();
          flexval_set_string (ptr, FV_ARRAY);
          ptr->type=FVT_ARRAY;
          prevline=linenum;
          while ((shift=unserialize_iterator (shift, token, &linenum, &flags, error)))
            {
              if (flags&FVP_FLAGS_ERROR) { free (newdata); goto __fail_; }
              if (flags&FVP_FLAGS_ARRAY)
                {
                  //
                  flex_value_t *tmp;
                  sprintf (dummy, "[%s]", token);
                  tmp=flexval_unserialize_entry (dummy, prevline, error);
                  if (strcmp (error, ""))
                    {
                      if (tmp) flexval_free (tmp);
                      free (newdata);
                      goto __fail_;
                    }
                  flexval_array_append (ptr, tmp);
                  flexval_free (tmp);
                } else
                {
                  if (flags&FVP_FLAGS_STRING)
                    flexval_array_append_string (ptr, token); else
                    {
                      if (is_integer (token)) flexval_array_append_int   (ptr, flexval_atol  (token)); else
                      if (is_number  (token)) flexval_array_append_float (ptr, flexval_atolf (token)); else
                        flexval_array_append_string (ptr, token);
                    }
                }
              prevline=linenum;
            }

          free (newdata);
        } else {
          trim (shift, dummy);
          // Check for da extra information
          if (strcmp (dummy, "")) { strcpy (error, "Extra information"); goto __fail_; }

          // Allocate new value
          ptr=flexval_alloc ();
          
          // Smart setting of value
          if (flags&FVP_FLAGS_STRING)
            flexval_set_string (ptr, token); else
          {
            if (is_integer (token)) flexval_set_int   (ptr, flexval_atol  (token)); else
            if (is_number  (token)) flexval_set_float (ptr, flexval_atolf (token)); else
              flexval_set_string (ptr, token);
          }
        }
      prevline=linenum;
    }

  if (flags&FVP_FLAGS_ERROR) goto __fail_;

  return ptr;

__fail_:
  if (__error) sprintf (__error, "%s at line %d", error, linenum);
  return ptr;
}

flex_value_t*
flexval_unserialize                (char *__data)
{
  return flexval_unserialize_entry (__data, 0, 0);
}

////////////////////////////////////////
//

void            // Set value as string
flexval_set_string                 (flex_value_t *__self, char *__str)
{
  char flexval_buffer[65536];
  if (!__self) return;
  if (!__str) return;
  SAFE_FREE (__self->pchar);
  snprintf (flexval_buffer, 65535, "%s", __str);
  __self->pchar = malloc (strlen (flexval_buffer)+1);
  flexval_free_array (__self);
  strcpy (__self->pchar, flexval_buffer);
  flexval_eval_from_string (__self);
}

void            // Set value as integer
flexval_set_int                    (flex_value_t *__self, long __value)
{
  if (!__self) return;
  flexval_free_array (__self);
  __self->integer = __value;
  flexval_eval_from_int (__self);
}

void            // Set value as float
flexval_set_float                  (flex_value_t *__self, double __value)
{
  if (!__self) return;
  flexval_free_array (__self);
  __self->real = __value;
  flexval_eval_from_float (__self);
}

flex_value_t*   // Full duplicating of element
flexval_dup                        (flex_value_t *__self)
{
  flex_value_t *ptr;

  if (!__self) return 0;

  ptr=flexval_alloc ();

  // Default dumping
  flexval_set_string (ptr, __self->pchar);
  ptr->integer = __self->integer;
  ptr->real    = __self->real;

  FLEXVAL_ARRAY_LENGTH (ptr)=0;
  FLEXVAL_ARRAY_DATA (ptr)=0;

  ptr->type=__self->type;

  if (__self->type==FVT_ARRAY) // Dumping of array
    {
      int i, n;
      n=FLEXVAL_ARRAY_LENGTH (__self);

      FLEXVAL_ARRAY_LENGTH (ptr)=n;
      FLEXVAL_ARRAY_DATA (ptr)=malloc (sizeof (flex_value_t*)*(n+1));

      for (i=0; i<n; i++)
        {
          // Duplicatimg of eacj element in array
          FLEXVAL_ARRAY_ELEM (ptr, i)=flexval_dup (FLEXVAL_ARRAY_ELEM (__self, i));
        }

      FLEXVAL_ARRAY_ELEM (ptr, n)=0;
    }

  return ptr;
}

void
flexval_set_array                  (flex_value_t *__self, flex_value_t **__data)
{
  int i, n=0;
  flexval_zerolize (__self);

  strcpy (__self->pchar, FV_ARRAY);

  while (__data[n]) n++; // Calculate length of assignment 

  // Alloc memory and store length
  FLEXVAL_ARRAY_DATA (__self)=malloc (sizeof (flex_value_t*)*(n+1));
  FLEXVAL_ARRAY_LENGTH (__self)=n;

  // Duplicate data
  for (i=0; i<n; i++)
    {
      // Duplicatin each element from assignment array
      FLEXVAL_ARRAY_ELEM (__self, i)=flexval_dup (__data[i]);
    }

  FLEXVAL_ARRAY_ELEM (__self, n)=0;

  __self->type=FVT_ARRAY;
}

void
flexval_set_serialized_array_entry (flex_value_t *__self, char *__data, int __base_line, char *__error)
{
  flex_value_t *ptr=0;
  ptr=flexval_unserialize_entry (__data, __base_line, __error);

  FLEXVAL_ARRAY_DATA   (__self)=FLEXVAL_ARRAY_DATA (ptr);
  FLEXVAL_ARRAY_LENGTH (__self)=FLEXVAL_ARRAY_LENGTH (ptr);

  flexval_set_array (__self, (flex_value_t**)FLEXVAL_ARRAY_DATA (ptr));

  SAFE_FREE (ptr->pchar);
}

char*           // Get string
flexval_get_string                 (flex_value_t *__self)
{
  if (__self) return __self->pchar;
  return "";
}

long            // Get integer
flexval_get_int                    (flex_value_t *__self)
{
  if (__self) return __self->integer;
  return 0;
}

double          // Get float
flexval_get_float                  (flex_value_t *__self)
{
  if (__self) return __self->real;
  return 0;
}

flex_value_t**  // Get array
flexval_get_array                  (flex_value_t *__self)
{
  if (__self) return (flex_value_t**)FLEXVAL_ARRAY_DATA (__self);
  return 0;
}

flex_value_t*   // Get element of array
flexval_get_array_elem             (flex_value_t *__self, int __index)
{
  if (__self && __index>=0 && __index<FLEXVAL_ARRAY_LENGTH (__self)) return (flex_value_t*)FLEXVAL_ARRAY_ELEM (__self, __index);
  return 0;
}

long            // Get integer value of array;s element
flexval_get_array_int              (flex_value_t *__self, int __index)
{
  return flexval_get_int (flexval_get_array_elem (__self, __index));
}

double          // Get float value of array;s element
flexval_get_array_float            (flex_value_t *__self, int __index)
{
  return flexval_get_float (flexval_get_array_elem (__self, __index));
}

char*           // Get string value of array;s element
flexval_get_array_string           (flex_value_t *__self, int __index)
{
  return flexval_get_string (flexval_get_array_elem (__self, __index));
}

flex_value_t**  // Get array value of array;s element
flexval_get_array_array            (flex_value_t *__self, int __index)
{
  return flexval_get_array (flexval_get_array_elem (__self, __index));
}

void            // Zerolization of value
flexval_zerolize                   (flex_value_t *__self)
{
  flexval_set_string (__self, "");
}

void            // Set the value as `undefined`
flexval_set_undefined              (flex_value_t *__self)
{
  __self->type=FVT_UNDEFINED;
}

static void
flexval_create_entry               (flex_value_t *__self)
{
  __self->pchar = 0;
  FLEXVAL_ARRAY_LENGTH (__self)=0;
  FLEXVAL_ARRAY_DATA (__self)=0;
  flexval_zerolize (__self);
  flexval_set_undefined (__self);
}

void            // Creating of flexvalue
flexval_create                     (flex_value_t *__self)
{
  __self->allocated=0;
  flexval_create_entry (__self);
}

void            // Freeing of flexvalue
flexval_free                       (flex_value_t *__self)
{
  if (!__self) return;
  SAFE_FREE (__self->pchar);
  flexval_free_array (__self);
  if (__self->allocated) free (__self);
}

static void
flexval_free_array_entry           (flex_value_t *__self, int __interior)
{
  int i;
  if (!__self) return;

  if (__interior)
    {
      SAFE_FREE (__self->pchar);
      if (__self->allocated) free (__self);
    }

  for (i=0; i<FLEXVAL_ARRAY_LENGTH (__self); i++)
    {
      flexval_free_array_entry (FLEXVAL_ARRAY_ELEM (__self, i), __interior+1);
    }

  SAFE_FREE (FLEXVAL_ARRAY_DATA (__self));
  FLEXVAL_ARRAY_DATA (__self)=0;
}

static void
flexval_free_array                 (flex_value_t *__self)
{
  flexval_free_array_entry (__self, 0);
}

int             // Value is truth?
flexval_is_truth                   (flex_value_t *__self)
{
  return is_truth (flexval_get_string (__self));
}


////////////////////////////////////////
// FlexVal ARRAY stuff

void
flexval_array_append               (flex_value_t *__self, flex_value_t *__element)
{
  int i, n;

  flex_value_t *ptr;
  flex_value_t **sdata;
  if (!__self || !__element) return;

  __self->integer=__self->real=0;
  strcpy (__self->pchar, FV_ARRAY);
  __self->type=FVT_ARRAY;

  ptr=flexval_dup (__element);

  n=FLEXVAL_ARRAY_LENGTH (__self);

  sdata=(flex_value_t**)FLEXVAL_ARRAY_DATA (__self);
  FLEXVAL_ARRAY_DATA (__self)=malloc (sizeof (flex_value_t*)*(n+2));

  for (i=0; i<n; i++)
    FLEXVAL_ARRAY_ELEM (__self, i)=sdata[i];

  FLEXVAL_ARRAY_ELEM (__self, n)=ptr;
  FLEXVAL_ARRAY_ELEM (__self, n+1)=0;

  FLEXVAL_ARRAY_LENGTH (__self)++;

  SAFE_FREE (sdata);
}

void
flexval_array_append_int           (flex_value_t *__self, long __data)
{
  flex_value_t data;
  flexval_create (&data);
  flexval_set_int (&data, __data);
  flexval_array_append (__self, &data);
  flexval_free (&data);
}

void
flexval_array_append_float         (flex_value_t *__self, double __data)
{
  flex_value_t data;
  flexval_create (&data);
  flexval_set_float (&data, __data);
  flexval_array_append (__self, &data);
  flexval_free (&data);
}

void
flexval_array_append_string        (flex_value_t *__self, char *__data)
{
  flex_value_t data;
  flexval_create (&data);
  flexval_set_string (&data, __data);
  flexval_array_append (__self, &data);
  flexval_free (&data);
}

void
flexval_array_append_array         (flex_value_t *__self, flex_value_t **__data)
{
  flex_value_t data;
  flexval_create (&data);
  flexval_set_array (&data, __data);
  flexval_array_append (__self, &data);
  flexval_free (&data);
}

////////////////////////////////////////
//

long            // Smart converting string to long
flexval_atol                       (char *__self)
{

  // TODO: Maybe move this stuff onto utils.c?

  int   sign;
  long  val;
  if (!__self) return 0;
	  
  if (*__self == '-')
    {
      sign = -1;
      __self++;
    } else sign = 1;
	
  val = 0;

  // check for HEX	
  if (*__self == '0' && (__self[1] == 'x' || __self[1] == 'X'))
    {
      __self += 2;
      for (;;)
	    {
          if (*__self >= '0' && *__self <= '9')
            val = (val << 4) + *__self - '0'; else
          if (*__self >= 'a' && *__self <= 'f')
            val = (val << 4) + *__self - 'a' + 10; else
          if (*__self >= 'A' && *__self <= 'F')
            val = (val << 4) + *__self - 'A' + 10; else
            return sign*val;
          __self++;
	    }
    }
 
  // check for character	
  if (*__self == '\'')
    return sign * __self[1];
  
  for (;;)
    {
      if (*__self < '0' || *__self > '9')
        return sign * val;
      val = val*10 + *__self - '0';
      __self++;
    }
}

double          // Smart converting string to float
flexval_atolf                      (char *__self)
{

  // TODO: Maybe move this stuff onto utils.c?

  int      sign=0;
  double   val;
  long     decimal;
  long     total;
  if (!__self) return 0.0;
  val = 0;  
  if (*__self == '-')
    {
      sign = -1;
      __self++;
    } else sign = 1;
  // check for HEX	
  if (*__self == '0' && (__self[1] == 'x' || __self[1] == 'X'))
    {
      *__self += 2;
      for (;;)
	    {
          if (*__self >= '0' && *__self <= '9')
            val = val*16 + *__self - '0'; else
          if (*__self >= 'a' && *__self <= 'f')
            val = val*16 + *__self - 'a' + 10; else
          if (*__self >= 'A' && *__self <= 'F')
            val = val*16 + *__self - 'A' + 10; else
            return sign*val;
          __self++;
	    }
    }
  // check for character	
  if (*__self == '\'')
    return sign * __self[1];
  decimal = -1;
  total = 0;
  for (;;)
    {
      if (*__self == '.')
        {
          decimal = total;
          __self++;
          continue;
        }
      if (*__self < '0' || *__self > '9')
        break;
      val = val*10 + *__self - '0';
      __self++;
      total++;
    }
	if (decimal==-1) return sign*val;
  while (total > decimal)
    {
      val /= 10;
      total--;
    }
	
  return sign*val;
}
