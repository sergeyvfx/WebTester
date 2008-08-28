/*
 *
 * =============================================================================
 *  hive.h
 * =============================================================================
 *
 *  HIVE datastruct (tree) stuff.
 *
 *  Written (by Nazgul) under GPL
 *
*/


#ifndef _hive_h_
#define _hive_h_

#include <libwebtester/flexval.h>
#include <libwebtester/dynastruc.h>

#define HPF_UNDEFINED 0x00000001
#define HPF_STRING    0x00000002
#define HPF_ARRAY     0x00000004

#define hive_header(self)              ((*self).header)
#define hive_header_name(self)         ((*self).header.name)
#define hive_header_value(self)        ((*self).header.value)
#define hive_header_int_value(self)    (flexval_get_int    (&(*self).header.value))
#define hive_header_float_value(self)  (flexval_get_float  (&(*self).header.value))
#define hive_header_string_value(self) (flexval_get_string (&(*self).header.value))

typedef struct
{
  char *name;
  flex_value_t value;
} conf_variable_t;

typedef struct
{
  int index;
  conf_variable_t header;
  dynastruc_t *nodes;
  void *owner;
  void *parent;
} hive_item_t;

void
hive_dyna_deleter                  (void *__self);

int
hive_key_comparator                (void *__l, void *__r);

hive_item_t*
hive_spawn_new_node                (hive_item_t *__parent, hive_item_t *__owner, char *__name, char *__value, int __flags);

int
hive_add_variable	                 (hive_item_t *__self, char *__name, char *__value, int __flags);

int
hive_parse_buf                     (char *__data, dynastruc_t **__self, char *__error, char *__cur_dur);

int
hive_parse_file                    (char *__fn, dynastruc_t **__self, char *__error);

dyna_item_t*
hive_find_item                     (dynastruc_t* __self, char* __key);

flex_value_t*
hive_open_key                      (dynastruc_t *__self, char* __key);

void
hive_free_tree                     (dynastruc_t *__self);

hive_item_t*
hive_node_by_index                 (hive_item_t *__self, int __index);

hive_item_t*
hive_next_sibling                  (hive_item_t *__self);

hive_item_t*
hive_prev_sibling                  (hive_item_t *__self);

hive_item_t*
hive_first_child                   (hive_item_t *__self);

hive_item_t*
hive_last_child                    (hive_item_t *__self);

void
hive_dump_to_buf                   (dynastruc_t *__self, char **__buf);

int
hive_dump_to_file                  (dynastruc_t *__self, char *__fn);

#endif
