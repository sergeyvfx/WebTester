/*
 *
 * ================================================================================
 *  conf.h
 * ================================================================================
 *
 *  Configuration stuff
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_conf_h_
#define _wt_conf_h_

#include <libwebtester/flexval.h>
#include <libwebtester/hive.h>
#include <libwebtester/util.h>

#define CONFIG_OPEN_KEY(__val,__key) \
  if (config_open_key (__key)) __val=config_open_key (__key);

#define CONFIG_KEY_EXISTS(__key) \
  (config_open_key (__key))

#define CONFIG_INT_KEY(__val,__key) \
  if (config_open_key (__key)) __val=flexval_get_int (config_open_key (__key));

#define CONFIG_FLOAT_KEY(__val,__key) \
  if (config_open_key (__key)) __val=flexval_get_float (config_open_key (__key));

#define CONFIG_PCHAR_KEY(__val,__key) \
  if (config_open_key (__key)) strcpy (__val, flexval_get_string (config_open_key (__key)));

#define CONFIG_BOOL_KEY(__val,__key) \
  if (config_open_key (__key)) __val=is_truth (flexval_get_string (config_open_key (__key)));

int
config_init                        (char *__fn);

void
config_done                        (void);

void
config_uload                       (void);

int
config_load                        (void);

char*
config_get_error                   (void);

hive_item_t*
config_find_item                   (char *__item);

flex_value_t*
config_open_key                    (char *__key);

void
config_dump_to_buf                 (char **__buf);

void
config_dump_to_file                (char *__fn);

#endif
