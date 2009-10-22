/**
 * WebTester Server - server of on-line testing system
 *
 * Configuration stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _wt_conf_h_
#define _wt_conf_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/flexval.h>
#include <libwebtester/hive.h>
#include <libwebtester/util.h>

/********
 * Macrodefinitions
 */

#define CONFIG_OPEN_KEY(__val,__key) \
  if (config_open_key (__key)) __val=config_open_key (__key);

#define CONFIG_KEY_EXISTS(__key) \
  (config_open_key (__key))

#define CONFIG_INT_KEY(__val,__key) \
  if (config_open_key (__key)) \
    { \
      __val=flexval_get_int (config_open_key (__key)); \
    }

#define CONFIG_FLOAT_KEY(__val,__key) \
  if (config_open_key (__key)) \
    { \
      __val=flexval_get_float (config_open_key (__key)); \
    }

#define CONFIG_PCHAR_KEY(__val,__key) \
  if (config_open_key (__key)) \
    { \
      strcpy (__val, flexval_get_string (config_open_key (__key))); \
    }

#define CONFIG_BOOL_KEY(__val,__key) \
  if (config_open_key (__key)) \
    { \
      __val=is_truth (flexval_get_string (config_open_key (__key))); \
    }

/********
 *
 */

/* Initialize configuration stuff */
int
config_init (char *__fn);

/* Uninitialize configuration stuff */
void
config_done (void);

/* Get config error */
char*
config_get_error (void);

/* Find hive item in configuration tree */
hive_item_t*
config_find_item (char *__item);

/* Get flexval key of config tree */
flex_value_t*
config_open_key (char *__key);

/* Dump configuration tree to buffer */
void
config_dump_to_buf (char **__buf);

/* Dump config tree to file */
void
config_dump_to_file (char *__fn);

END_HEADER

#endif
