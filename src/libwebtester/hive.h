/**
 * WebTester Server - server of on-line testing system
 *
 * HIVE datastruct (tree) stuff.
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */


#ifndef _hive_h_
#define _hive_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/flexval.h>
#include <libwebtester/dynastruc.h>

/********
 * Constants
 */

#define HPF_UNDEFINED 0x00000001
#define HPF_STRING    0x00000002
#define HPF_ARRAY     0x00000004

#define hive_header(self)              ((*self).header)
#define hive_header_name(self)         ((*self).header.name)
#define hive_header_value(self)        ((*self).header.value)

#define hive_header_int_value(self) \
  (flexval_get_int    (&(*self).header.value))

#define hive_header_float_value(self)  \
  (flexval_get_float  (&(*self).header.value))

#define hive_header_string_value(self) \
  (flexval_get_string (&(*self).header.value))

/********
 * Type definitions
 */

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

/********
 *
 */

/* Deleter for hive tree node */
void
hive_dyna_deleter (void *__self);

/* Parse buffer */
int
hive_parse_buf (char *__data, dynastruc_t **__self,
                char *__error, char *__cur_dur);

/* Parse file to build hive tree */
int
hive_parse_file (char *__fn, dynastruc_t **__self, char *__error);

/* Find item in hive tree */
dyna_item_t*
hive_find_item (dynastruc_t* __self, char* __key);

/* Open hive tree key */
flex_value_t*
hive_open_key (dynastruc_t *__self, char* __key);

/* Free hive tree */
void
hive_free_tree (dynastruc_t *__self);

/* Get hive item by index */
hive_item_t*
hive_node_by_index (hive_item_t *__self, int __index);

/* Get next sibling node */
hive_item_t*
hive_next_sibling (hive_item_t *__self);

/* Get previous sibling node */
hive_item_t*
hive_prev_sibling (hive_item_t *__self);

/* Get first child node */
hive_item_t*
hive_first_child (hive_item_t *__self);

/* Get last child node */
hive_item_t*
hive_last_child (hive_item_t *__self);

/* Dump hive tree to buffer */
void
hive_dump_to_buf (dynastruc_t *__self, char **__buf);

/* Dump hive tree to file */
int
hive_dump_to_file (dynastruc_t *__self, char *__fn);

END_HEADER

#endif
