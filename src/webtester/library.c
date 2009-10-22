/**
 * WebTester Server - server of on-line testing system
 *
 * Plugins and testing modules stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include <libwebtester/plugin.h>
#include <libwebtester/fs.h>
#include <libwebtester/dynastruc.h>
#include <libwebtester/strlib.h>

#include "autoinc.h"
#include "task.h"

#include "library.h"

#include <dlfcn.h>
#include <malloc.h>

#define MAX_LIBRARY_NAME 1024

#define LOADSYM(lib,proc,type,sym) \
  { \
    PLUGIN_SYM(lib,proc,type,sym);\
    if (!proc) \
      { \
        core_set_last_error ("No symbol %s found", sym); \
        return -1; \
      } \
  }

/****
 * Some type defenitions
 */

typedef
BOOL (*module_start_for_testing) (wt_task_t*, char*);

typedef struct
{
  char name[MAX_LIBRARY_NAME];
  int id;

  struct
  {
    module_start_for_testing StartForTesting;
  } procs;

} wt_module_t;

/****
 *
 */

static dynastruc_t *modules = NULL;

/**
 * Compare modules by ID
 *
 * @param __data - library from dyna
 * @param __id - id of needed library
 * @return non-zero if library is wanted, zero otherwise
 */
static int
module_id_comparator (void *__data, void *__id)
{
  wt_module_t *module = __data;
  return ITOL (module->id) == (long) __id;
}

/**
 * Compare modules by name
 *
 * @param __data - library from dyna
 * @param __name - name of needed library
 * @return non-zero if library is wanted, zero otherwise
 */
static int
module_name_comparator (void *__data, void *__name)
{
  wt_module_t *module = __data;
  return !strcmp (module->name, (char *)__name);
}

/**
 * Load all wanted symbols from library
 *
 * @param __data - descriptor of module
 * @param __name - name of module
 */
static int
load_module_symbols (wt_module_t *__data, const char *__name)
{
  LOADSYM (__name, __data->procs.StartForTesting, module_start_for_testing,
           "StartForTesting");

  return 0;
}

/**
 * Spawn new module descriptor
 *
 * @param __name - name of module
 * @param __id - id of module
 * @return new module descriptor
 */
static wt_module_t*
spawn_new_module (const char *__name, int __id)
{
  wt_module_t *ptr = malloc (sizeof (wt_module_t));
  ptr->id = __id;
  strcpy (ptr->name, __name);

  if (load_module_symbols (ptr, __name))
    {
      free (ptr);
      ptr = NULL;
    }

  return ptr;
}

/**
 * Load module by name and ID
 *
 * @param __name - name of module to load
 * @param __id - id of nodule
 * @return zero on success, non-zero otherwise
 */
static int
wt_load_single_module (const char *__name, int __id)
{
  wt_module_t *ptr;
  char fname[4096], *full = 0;
  char *error;

  /* Attach prefix and suffix to libname */
  snprintf (fname, BUF_SIZE (fname), "%s%s%s", LIB_PREFIX, __name, LIB_SUFFIX);
  strlowr (fname, fname);
  full = get_full_path (fname);

  core_print (MSG_INFO, "        Loading module %s with id %d...",
              __name, __id);

  /* Load module as plugin */
  if (plugin_load (full))
    {
      goto __fail_;
    }

  ptr = spawn_new_module (__name, __id);
  if (!ptr)
    {
      plugin_unload_with_fn (full);
      goto __fail_;
    }

  dyna_append (modules, ptr, 0);

  plugin_activate_by_fn (full);

  SAFE_FREE (full);

  CMSG_OK ();
  return 0;

__fail_:
  CMSG_FAILED ();
  if (strcmp (core_get_last_error (), ""))
    {
      core_print (MSG_WARNING, "          %s\n", core_get_last_error ());
    }

  error = plugin_load_error ();

  if (error && strcmp (error, ""))
    {
      core_print (MSG_WARNING, "          %s\n", error);
    }

  SAFE_FREE (full);
  return -1;
}

/**
 * Load one plugin
 *
 * @param __lib - name of library to load
 * @return zero on success, non-zero otherwise
 */
static int
load_single_plugin (char *__lib)
{
  char fname[4096], *full = 0;

  /* Attach prefix and suffix to libname */
  snprintf (fname, BUF_SIZE (fname), "%s%s%s", LIB_PREFIX, __lib, LIB_SUFFIX);
  strlowr (fname, fname);
  full = get_full_path (fname);

  core_print (MSG_INFO, "        Loading plugin %s...", __lib);

  if (plugin_load (full))
    {
      CMSG_FAILED ();
      free (full);
      return -1;
    }

  CMSG_OK ();

  plugin_activate_by_fn (full);

  free (full);
  return 0;
}

/********
 * User's backend
 */

/**
 * Load modules for testing stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
wt_load_modules (void)
{
  char *name;
  int id;
  modules = dyna_create ();
  hive_item_t *item = config_find_item ("Server/Modules");

  if (!item)
    {
      return 0;
    }

  item = hive_first_child (item);
  while (item)
    {
      name = hive_header_name (item);
      id = hive_header_int_value (item);
      wt_load_single_module (name, id);

      item = hive_next_sibling (item);
    }

  return 0;
}

/**
 * Unload testing stuff's modules
 */
void
wt_unload_modules (void)
{
  /* Just destroy dynalist */
  /* Loaded symbols will be freed when plugins' stuff wll be stopped */

  dyna_destroy (modules, 0);
}

/**
 * Send task to module for testing
 *
 * @param __self - task to send
 * @param __error - buffer for error dexcription
 * @return zero on success, non-zero otherwise
 */
int
wt_module_send_for_testing (wt_task_t *__self, char *__error)
{
  wt_module_t *lib;
  dyna_item_t *item;

  dyna_search_reset (modules);

  item = dyna_search (modules, (void*) ITOL (TASK_CURLID (__self)), 0,
                      module_id_comparator);

  if (!item)
    {
      strcpy (__error, "No such module");
      return -1;
    }

  lib = dyna_data (item);
  if (!lib || !lib->procs.StartForTesting) /* But why??? */
    {
      strcpy (__error, "No StartForTesting entrypoint in module");
      return -1;
    }

  if (!lib->procs.StartForTesting (__self, __error))
    {
      return -1;
    }

  return 0;
}

/**
 * Return testing module name
 *
 * @return module name
 */
char*
wt_module_name (int __lid)
{
  wt_module_t *lib;
  dyna_item_t *item;
  dyna_search_reset (modules);
  item = dyna_search (modules, (void*) ITOL (__lid), 0, module_id_comparator);
  lib = dyna_data (item);

  if (!item || !lib || !lib->name)
    {
      return "Undefined";
    }

  return lib->name;
}

/**
 * Load all plugins
 *
 * @return zero on success, non-zero otherwise
 */
int
wt_load_plugins (void)
{
  char *name;
  hive_item_t *item = config_find_item ("Server/Plugins");

  if (!item)
    {
      return 0;
    }

  item = hive_first_child (item);
  while (item)
    {
      name = hive_header_name (item);
      load_single_plugin (name);

      item = hive_next_sibling (item);
    }
  return 0;
}

/**
 * Returns ID of testing module
 *
 * @param __name - name of module to get ID of
 * @return ID of needed module
 */
int
wt_module_id (const char *__name)
{
  wt_module_t *lib;
  dyna_item_t *item;
  dyna_search_reset (modules);
  item = dyna_search (modules, (void*) __name, 0, module_name_comparator);
  lib = dyna_data (item);

  if (!item || !lib || !lib->name)
    {
      return -1;
    }

  return lib->id;
}
