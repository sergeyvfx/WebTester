/**
 * WebTester Server - server of on-line testing system
 *
 * Configuration stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "conf.h"
#include "hive.h"
#include "dynastruc.h"
#include "mutex.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>

#define LOCK   mutex_lock (mutex)
#define UNLOCK mutex_unlock (mutex)

static dynastruc_t *config = 0;
static char *config_error = 0;
static char config_file[1024];
static mutex_t mutex = 0;

/********
 *
 */

/**
 * Load configuration file
 *
 * @return zero on success, non-zero otherwise
 */
static int
config_load (void)
{
  return hive_parse_file (config_file, &config, config_error);
}

/**
 * Unload loaded data structure
 */
static void
config_uload (void)
{
  if (!config)
    {
      return;
    }
  hive_free_tree (config);
  config = 0;
}

/********
 * User's backend
 */

/**
 * Initialize configuration stuff
 *
 * @param __fn - name of configuration file
 * @return zero on success, non-zero otherwise
 */
int
config_init (char *__fn)
{
  config_error = malloc (65536);
  strcpy (config_error, "");
  strcpy (config_file, __fn);
  mutex = mutex_create ();
  if (config_load ())
    {
      return -1;
    }
  return 0;
}

/**
 * Uninitialize configuration stuff
 */
void
config_done (void)
{
  mutex_free (mutex);
  if (config_error)
    {
      free (config_error);
    }
  config_uload ();
}

/**
 * Get config error
 *
 * @return config error
 */
char*
config_get_error (void)
{
  return config_error;
}

/**
 * Find hive item in configuration tree
 *
 * @param - name of item (absolute path)
 * @return hive item descriptor
 */
hive_item_t*
config_find_item (char *__name)
{
  dyna_item_t *item;
  char buf[65536];
  sprintf (buf, "root/%s", __name);

  LOCK;
    item = hive_find_item (config, buf);
  UNLOCK;

  if (!item)
    {
      return 0;
    }

  return item->data;
}

/**
 * Get flexval key of config tree
 *
 * @param __key - path to key
 * @return flexval key
 */
flex_value_t*
config_open_key (char *__key)
{
  flex_value_t *res;
  LOCK;
    res = hive_open_key (config, __key);
  UNLOCK;
  return res;
}

/**
 * Dump configuration tree to buffer
 *
 * @param __buf - buffer, which will be allocated
 */
void
config_dump_to_buf (char **__buf)
{
  hive_dump_to_buf (config, __buf);
}

/**
 * Dump config tree to file
 *
 * @param __fn - name of file
 */
void
config_dump_to_file (char *__fn)
{
  hive_dump_to_file (config, __fn);
}
