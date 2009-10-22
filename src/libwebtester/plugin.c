/**
 * WebTester Server - server of on-line testing system
 *
 * Plugins support stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "plugin.h"
#include "fs.h"
#include "dynastruc.h"
#include "core.h"

#include <malloc.h>
#include <dlfcn.h>
#include <string.h>

#define ERROR(__err) \
  strcpy (error, __err)

static dynastruc_t *plugins_registered = 0;
static char error[65536];

/**
 * Comparator for searching plugin by it's name
 *
 * @param __data - current plugin
 * @param __name - name of wanted plugn
 * @return non-zero if plugin founf, zero otherwise
 */
static int
plugin_search_by_name_comparator (void *__data, void *__name)
{
  plugin_t *plugin = __data;
  return !strcmp (__name, plugin->name);
}

/**
 * Comparator for searching plugin by it's file name
 *
 * @param __data - current plugin
 * @param __name - file name of wanted plugn
 * @return non-zero if plugin founf, zero otherwise
 */
static int
plugin_search_by_fn_comparator (void *__data, void *__name)
{
  plugin_t *plugin = __data;
  return !strcmp (__name, plugin->fn);
}

/**
 * Comparator for searching plugin by it's handler
 *
 * @param __data - current plugin
 * @param __name - handler of wanted plugn
 * @return non-zero if plugin founf, zero otherwise
 */
static int
plugin_search_by_ptr_comparator (void *__left, void *__right)
{
  return __left == __right;
}

/**
 * Call `onload` method of plugin
 *
 * @param __self - plugin which are loading
 * @return zero on success, non-zero otherwise
 */
static int
plugin_call_onload (plugin_t *__self)
{
  if (__self->info.plugin_load)
    {
      return __self->info.plugin_load (__self);
    }

  return 0;
}

/**
 * Call `onunload` method of plugin
 *
 * @param __self - plugin which are unloading
 * @return zero on success, non-zero otherwise
 */
static int
plugin_call_onunload (plugin_t *__self)
{
  if (__self->info.plugin_unload)
    {
      return __self->info.plugin_unload (__self);
    }

  return 0;
}

/**
 * Call `activate` method of plugin
 *
 * @param __self - plugin which are activating
 * @return zero on success, non-zero otherwise
 */
static int
plugin_call_activate (plugin_t *__self)
{
  if (__self->info.plugin_activate)
    return __self->info.plugin_activate (__self);
  return 0;
}

/**
 * Call `deactivate` method of plugin
 *
 * @param __self - plugin which are deactivating
 * @return zero on success, non-zero otherwise
 */
static int
plugin_call_deactivate (plugin_t *__self)
{
  if (__self->info.plugin_activate)
    {
      return __self->info.plugin_deactivate (__self);
    }

  return 0;
}

/**
 * Get plugin by it's library file name
 *
 * @param __fn - name of library
 * @return descriptor of wanted plugin
 */
static plugin_t*
plugin_by_fn (const char *__fn)
{
  dyna_item_t *item;

  if (!plugins_registered || !__fn)
    {
      return 0;
    }

  dyna_search_reset (plugins_registered);
  item = dyna_search (plugins_registered, (void*)__fn, 0,
                      plugin_search_by_fn_comparator);

  if (!item)
    {
      return NULL;
    }

  return dyna_data (item);
}

/**
 * Load needed symbols from plugin library
 *
 * @param __self - descriptor of plugin for which loading will be applied
 * @return zero on success, non-zero otherwise
 */
static int
plugin_load_symbols (plugin_t *__self)
{
  __self->dl = dlopen (__self->fn, RTLD_LAZY);

  if (!__self->dl)
    {
      ERROR (dlerror ());
      return -1;
    }

  __self->procs.Init = dlsym (__self->dl, PLUGIN_INIT_PROC);

  return 0;
}

/**
 * Create new plugin descriptor
 *
 * @param __fn - name of library t ouse
 * @return new plugin descriptor
 */
plugin_t*
plugin_new (const char *__fn)
{
  char *full = get_full_path (__fn);
  plugin_t *plugin;
  MALLOC_ZERO (plugin, sizeof (plugin_t));
  strcpy (plugin->fn, full);
  plugin_load_symbols (plugin);
  free (full);
  return plugin;
}

/**
 * Free plugin descriptor
 *
 * @param __plugin - plugin to free
 */
static void
plugin_free (plugin_t *__self)
{
  if (!__self)
    {
      return;
    }

  if (__self->dl)
    {
      dlclose (__self->dl);
    }

  free (__self);
}

/**
 * Onload plugin handler
 *
 * @oaran __self - descriptor of loading plugin
 * @return zero on success, non-zero otherwise
 */
static int
plugin_on_load (plugin_t *__self)
{
  if (__self->procs.Init)
    __self->procs.Init (__self);
  else
    {
      ERROR ("No Init function");
      return -1;
    }
  return plugin_call_onload (__self);
}

/**
 * Probe plugin loading
 *
 * @param __self - descriptor of plugin to brobe
 * @return zero on success, non-zero otherwise
 */
static int
plugin_probe (plugin_t *__self)
{
  if (!__self)
    {
      return -1;
    }

  if (!__self->dl)
    {
      return -1;
    }

  if (!__self->procs.Init)
    {
      ERROR ("No Init function");
      return -1;
    }

  return 0;
}

/**
 * Dyna-deleter for plugin pool
 *
 * @param __self - descriptor of deleting plugin
 */
static void
plugin_dyna_deleter (void *__self)
{
  plugin_deactivate (__self);
  plugin_call_onunload (__self);
  plugin_free (__self);
}

/********
 * User's backend
 */

/**
 * Register plugin in pool
 *
 * @param __self - plugin to register
 * @return zero on success, non-zero otherwise
 */
int
plugin_register (const plugin_t *__self)
{
  if (plugins_registered &&
      dyna_length (plugins_registered) >= MAX_LOAD_PLUGINS)
    {
      return -1;
    }

  if (!plugins_registered)
    {
      /* Create pool of plugins */
      plugins_registered = dyna_create ();
    }

  if (!plugins_registered)
    {
      return -1;
    }

  return dyna_append (plugins_registered, (void*)__self, 0);
}

/**
 * Load plugin
 *
 * @param __fn - file name of library to use
 * @param zero on sucess, non-zero otherwise
 */
int
plugin_load (char *__fn)
{
  plugin_t *plugin;

  strcpy (error, "");
  plugin = plugin_new (__fn);

  if (plugin_probe (plugin) || plugin_on_load (plugin))
    {
      plugin_free (plugin);
      return -1;
    }

  return 0;
}

/**
 * Unload plugin
 *
 * @param __self - descriptor of plugin to unload
 * @return zero on success, non-zero otherwise
 */
int
plugin_unload (plugin_t *__self)
{
  dyna_item_t *item;

  if (!plugins_registered)
    {
      return 0;
    }

  dyna_search_reset (plugins_registered);
  item = dyna_search (plugins_registered, __self,
                      0, plugin_search_by_ptr_comparator);

  if (!item)
    {
      return -1;
    }

  dyna_delete (plugins_registered, item, plugin_dyna_deleter);

  return 0;
}

/**
 * Unload plugin by it's file name
 *
 * @param __fn - name of library to unload plugin
 * @return zero on success, non-zero otherwise
 */
int
plugin_unload_with_fn (char *__fn)
{
  dyna_item_t *item;

  if (!plugins_registered)
    {
      return 0;
    }

  dyna_search_reset (plugins_registered);
  item = dyna_search (plugins_registered, __fn,
                      0, plugin_search_by_fn_comparator);

  if (!item)
    {
      return -1;
    }

  dyna_delete (plugins_registered, item, plugin_dyna_deleter);
  return 0;
}

/**
 * Unload all plugins
 */
void
plugin_unload_all (void)
{
  dyna_destroy (plugins_registered, plugin_dyna_deleter);
  plugins_registered = NULL;
}

/**
 * Search plugin descitpor by plugin name
 *
 * @param __plugin_name - name of wanted plugin
 * @return plugin descitpor if found, NULL otherwise
 */
plugin_t*
plugin_search (const char *__plugin_name)
{
  dyna_item_t *item;

  if (!plugins_registered)
    {
      return NULL;
    }

  dyna_search_reset (plugins_registered);
  item = dyna_search (plugins_registered, (void*)__plugin_name, 0,
                      plugin_search_by_name_comparator);

  if (!item)
    {
      return NULL;
    }

  return item->data;
}

/**
 * Get plugins's library symbol
 *
 * @param __plugin_name - name of wanted plugin
 * @param __sym_name - name of wanted symbol
 * @return pointer to library sumbol of found, NULL otherwise
 */
void*
plugin_sym (const char *__plugin_name, const char *__sym_name)
{
  plugin_t *plugin = plugin_search (__plugin_name);

  if (!plugin)
    {
      core_set_last_error ("Plugin %s not loaded", __plugin_name);
      return NULL;
    }

  return dlsym (plugin->dl, __sym_name);
}

/**
 * Get last error occured in plugin stuff
 *
 * @param last occured error
 */
char*
plugin_load_error (void)
{
  if (strcmp (error, ""))
    {
      return error;
    }

  return dlerror ();
}

/**
 * Activate plugin
 *
 * @param __self - plugin to activate
 * @return zero on success, non-zero otherwise
 */
int
plugin_activate (plugin_t *__self)
{
  if (!__self)
    {
      return -1;
    }

  if (__self->activated)
    {
      return 0;
    }

  return plugin_call_activate (__self);
}

/**
 * Activate plugin by it's name
 *
 * @param __name - name of plugin to activate
 * @return zero on success, non-zero otherwise
 */
int
plugin_activate_by_name (const char *__name)
{
  return plugin_activate (plugin_search (__name));
}

/**
 * Activate plugin by it's library file name
 *
 * @param __fn - plugin's library file name to activate
 * @return zero on success, non-zero otherwise
 */
int
plugin_activate_by_fn (const char *__fn)
{
  return plugin_activate (plugin_by_fn (__fn));
}

/**
 * Deactivate plugin
 *
 * @param __self - plugin to deactivate
 * @return zero on success, non-zero otherwise
 */
int
plugin_deactivate (plugin_t *__self)
{
  if (!__self)
    {
      return -1;
    }

  if (!__self->activated)
    {
      return 0;
    }

  return plugin_call_deactivate (__self);
}

/**
 * Deactivate plugin by it's name
 *
 * @param __name - name of plugin to deactivate
 * @return zero on success, non-zero otherwise
 */
int
plugin_deactivate_by_name (const char *__name)
{
  return plugin_deactivate (plugin_search (__name));
}

/**
 * Deactivate plugin by it's name
 *
 * @param __fn - plugin's library file name to deactivate
 * @return zero on success, non-zero otherwise
 */
int
plugin_deactivate_by_fn (const char *__fn)
{
  return plugin_deactivate (plugin_by_fn (__fn));
}
