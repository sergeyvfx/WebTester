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

#include "conf.h"
#include "hive.h"
#include "dynastruc.h"
#include "mutex.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>

#define LOCK   mutex_lock (mutex)
#define UNLOCK mutex_unlock (mutex)

static dynastruc_t *config=0;
static char *config_error=0;
static char config_file[1024];
static mutex_t mutex=0;

int
config_init                        (char *__fn)
{
  config_error=malloc (65536);
  strcpy (config_error, "");
  strcpy (config_file, __fn);
  mutex=mutex_create ();
  if (config_load ())
    return -1;
  return 0;
}

void
config_done                        (void)
{
  mutex_free (mutex);
  if (config_error)
    free (config_error);
  config_uload ();
}

void
config_uload                       (void)
{
  if (!config) return;
  hive_free_tree (config);
  config=0;
}

int
config_load                        (void)
{
  return hive_parse_file (config_file, &config, config_error);
}

char*
config_get_error                   (void)
{
  return config_error;
}

hive_item_t*
config_find_item                   (char *__name)
{
  dyna_item_t *item;
  char buf[65536];
  sprintf (buf, "root/%s", __name);
  LOCK;
  item=hive_find_item (config, buf);
  UNLOCK;
  if (!item) return 0;
  return item->data;
}

flex_value_t*
config_open_key                    (char *__key)
{
  flex_value_t *res;
  LOCK;
  res=hive_open_key (config, __key);
  UNLOCK;
  return res;
}

void
config_dump_to_buf                 (char **__buf)
{
  hive_dump_to_buf (config, __buf);
}

void
config_dump_to_file                (char *__fn)
{
  hive_dump_to_file (config, __fn);
}
