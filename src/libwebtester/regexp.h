/*
 *
 * ================================================================================
 *  regexp.h
 * ================================================================================
 *
 *  Regular Expressions stuu
 *  Based on the GNU's PCRE
 *
 *  Written (by Nazgul) under General Public License.
 *
*/


#ifndef _regexp_h_
#define _regexp_h_

#define REGEXP_REPLACE_GLOBAL 0x00000001

#define REGEXP_START_ALLOCATION_LEN    65536
#define REGEXP_ALLOCATION_DELTA        1024
#define REGEXP_MAX_VECTOR_SIZE         128
#define REGEXP_MAX_SUBSTRING_LEN       65536

typedef struct
{
  void *handle;
  int userFlags;
  int flags;
} regexp_t;

typedef struct
{
   char ch;
   int code;
} regexp_modifer_t;

int
regexp_free                        (regexp_t *__self);

regexp_t
prepare_regexp                     (const char *__str, char *__error_message, int *__erroffset);

int
regexp_get_vector                  (regexp_t *__re, const char *__str, int *__ovector, int __ovector_size);

int
match_regexp                       (regexp_t *__re, const char *__str);

char*
regexp_replace                     (regexp_t *__re, const char *__mask, const char *__s);

////////////////////////////////////////
//

int
preg_match                         (const char *__regexp, const char *__str);

void
preg_replace                       (const char *__regexp, const char *__mask, char *__str);

#endif
