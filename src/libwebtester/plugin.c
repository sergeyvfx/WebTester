/*
 *
 * =============================================================================
 *  plugin.c
 * =============================================================================
 *
 *  PLugins support stuff
 *
 *  Written (by Nazgul) under GPL
 *
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

static dynastruc_t *plugins_registered=0;
static char error[65536];

int
plugin_search_by_name_comparator   (void *__data, void *__name)
{
  plugin_t *plugin=__data;
  return !strcmp (__name, plugin->name);
}

int
plugin_search_by_fn_comparator     (void *__data, void *__name)
{
  plugin_t *plugin=__data;
  return !strcmp (__name, plugin->fn);
}

int
plugin_search_by_ptr_comparator    (void *__left, void *__right)
{
  return __left==__right;
}

////
//

int
plugin_call_onload                 (plugin_t *__self)
{
  if (__self->info.plugin_load)
    return __self->info.plugin_load (__self);
  return 0;
}

int
plugin_call_onunload               (plugin_t *__self)
{
  if (__self->info.plugin_unload)
    return __self->info.plugin_unload (__self);
  return 0;
}

int
plugin_call_activate               (plugin_t *__self)
{
  if (__self->info.plugin_activate)
    return __self->info.plugin_activate (__self);
  return 0;
}

int
plugin_call_deactivate             (plugin_t *__self)
{
  if (__self->info.plugin_activate)
    return __self->info.plugin_deactivate (__self);
  return 0;
}

////
//

plugin_t*
plugin_by_fn                       (char *__fn)
{
  dyna_item_t *item;
  if (!plugins_registered || !__fn) return 0;
  dyna_search_reset (plugins_registered);
  item=dyna_search (plugins_registered, __fn, 0, plugin_search_by_fn_comparator);
	 if (!item)
    return 0;
  return dyna_data (item);
}


////
//

int
plugin_load_symbols                (plugin_t *__self)
{
  __self->dl=dlopen (__self->fn, RTLD_LAZY);
  if (!__self->dl)
    {
      ERROR (dlerror ());
      return -1;
    }
  __self->procs.Init=dlsym (__self->dl, PLUGIN_INIT_PROC);
  return 0;
}

plugin_t*
plugin_new                         (char *__fn)
{
  char *full=get_full_path (__fn);
  plugin_t *plugin=malloc (sizeof (plugin_t));
  memset (plugin, 0, sizeof (plugin));
  strcpy (plugin->fn, full);
  plugin_load_symbols (plugin);
  free (full);
  return plugin;
}

void
plugin_free                        (plugin_t *__self)
{
  if (!__self) return;
  if (__self->dl) dlclose (__self->dl);
  free (__self);
}

int
plugin_on_load                     (plugin_t *__self)
{
  if (__self->procs.Init)
    __self->procs.Init (__self); else
    {
      ERROR ("No Init function");
      return -1;
    }
  return plugin_call_onload (__self);
}

int
plugin_load                        (char *__fn)
{
  plugin_t *plugin;
  strcpy (error, "");
  plugin=plugin_new (__fn);
  if (plugin_probe (plugin) || plugin_on_load (plugin)) {
    plugin_free (plugin);
    return -1;
  }
  return 0;
}

int
plugin_probe                       (plugin_t *__self)
{
  if (!__self) return -1;
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

int
plugin_register                    (plugin_t *__self)
{
  if (plugins_registered && dyna_length (plugins_registered)>=MAX_LOAD_PLUGINS)
    return -1;
  if (!plugins_registered) plugins_registered=dyna_create ();
  if (!plugins_registered) return -1;
  return dyna_append (plugins_registered, __self, 0);
}

void
plugin_dyna_deleter                (void *__self)
{
  plugin_deactivate (__self);
  plugin_call_onunload (__self);
  plugin_free (__self);
}

int
plugin_unload_with_fn              (char *__fn)
{
  dyna_item_t *item;
  if (!plugins_registered) return 0;
  dyna_search_reset (plugins_registered);
  item=dyna_search (plugins_registered, __fn, 0, plugin_search_by_fn_comparator);
	 if (!item)
    return -1;
  dyna_delete (plugins_registered, item, plugin_dyna_deleter);
  return 0;
}

int
plugin_unload                      (plugin_t *__self)
{
  dyna_item_t *item;
  if (!plugins_registered) return 0;
  dyna_search_reset (plugins_registered);
  item=dyna_search (plugins_registered, __self, 0, plugin_search_by_ptr_comparator);
	 if (!item)
    return -1;
  dyna_delete (plugins_registered, item, plugin_dyna_deleter);
  return 0;
}

void
plugin_unload_all                  (void)
{
  dyna_destroy (plugins_registered, plugin_dyna_deleter);
  plugins_registered=0;
}

plugin_t*
plugin_search                      (char *__plugin_name)
{
  dyna_item_t *item;
  if (!plugins_registered) return 0;
  dyna_search_reset (plugins_registered);
  item=dyna_search (plugins_registered, __plugin_name, 0, plugin_search_by_name_comparator);
  if (!item) return 0;
  return item->data;
}

void*
plugin_sym                         (char *__plugin_name, char *__sym_name)
{
  plugin_t *plugin=plugin_search (__plugin_name);
  if (!plugin)
    {
      core_set_last_error ("Plugin %s not loaded", __plugin_name);
      return 0;
    }
  return dlsym (plugin->dl, __sym_name);
}

char*
plugin_load_error                  (void)
{
  if (strcmp (error, ""))
    return error;
  return dlerror ();
}

int
plugin_activate                    (plugin_t *__self)
{ 
  if (!__self)
     return -1;

  if (__self->activated)
    return 0;

  if (__self->info.plugin_activate)
    return __self->info.plugin_activate (__self);

  return 0;
}

int
plugin_activate_by_name            (char *__name)
{
  return plugin_activate (plugin_search (__name));
}

int
plugin_activate_by_fn              (char *__fn)
{
  return plugin_activate (plugin_by_fn (__fn));
}

int
plugin_deactivate                  (plugin_t *__self)
{ 
  if (!__self)
     return -1;

  if (!__self->activated)
    return 0;

  if (__self->info.plugin_deactivate)
    return __self->info.plugin_deactivate (__self);

  return 0;
}

int
plugin_deactivate_by_name          (char *__name)
{
  return plugin_deactivate (plugin_search (__name));
}

int
plugin_deactivate_by_fn            (char *__fn)
{
  return plugin_deactivate (plugin_by_fn (__fn));
}
