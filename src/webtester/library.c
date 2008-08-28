/*
 *
 * ================================================================================
 *  library.c - part of the WebTester Server
 * ================================================================================
 *
 *  WT plugins and testing modules stuff
 *
 *  Written (by Nazgul) under General Public License.
 *
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

////////
// Some type defenitions

typedef BOOL (*module_start_for_testing) (wt_task_t*, char*);

typedef struct {
  char  name[MAX_LIBRARY_NAME];
  int   id;
  
  struct {
    module_start_for_testing StartForTesting;
  } procs;
  
} wt_module_t;

////
//

static dynastruc_t *modules=NULL;

////////////////////////////////////////
// DYNA stuff

int             // Compare modules by ID
wt_module_id_comparator            (void *__data, void *__id)
{
  wt_module_t *module=__data;
  return ITOL (module->id)==(long)__id;
}

////////////////////////////////////////
//

static int      // Load all wanted wymvols from library
load_module_symbols                (wt_module_t *__data, char *__name)
{
  LOADSYM (__name, __data->procs.StartForTesting, module_start_for_testing, "StartForTesting");
  return 0;
}

static wt_module_t*
spawn_new_module                   (char *__name, int __id)
{
  wt_module_t *ptr=malloc (sizeof (wt_module_t));
  ptr->id=__id;
  strcpy (ptr->name, __name);
  if (load_module_symbols (ptr, __name))
    {
      free (ptr);
      ptr=0;
    }
  return ptr;
}

static int      // Load module by name and ID
wt_load_single_module              (char *__name, int __id)
{
  wt_module_t *ptr;
  char fname[65536], *full=0;
  char *error;

  // Attach prefix and suffix to libname
  sprintf (fname, "%s%s%s", LIB_PREFIX, __name, LIB_SUFFIX);
  strlowr (fname, fname);
  full=get_full_path (fname);

  core_print (MSG_INFO, "        Loading module %s with id %d...", __name, __id);

  // Load module as plugin
  if (plugin_load (full))
    goto __fail_;

  ptr=spawn_new_module (__name, __id);
  if (!ptr)
    {
      plugin_unload_with_fn (full);
      goto __fail_;
    }
  dyna_append (modules, ptr, 0);

  SAFE_FREE (full);
  CMSG_OK ();
  return 0;

__fail_:
  CMSG_FAILED ();
  if (strcmp (core_get_last_error (), ""))
    core_print (MSG_WARNING, "          %s\n", core_get_last_error ());
  error=plugin_load_error ();
  if (error && strcmp (error, ""))
    core_print (MSG_WARNING, "          %s\n", error);
  SAFE_FREE (full);
  return -1;
}

////////////////////////////////////////
//

int             // Load modules for testing stuff
wt_load_modules                    (void)
{
  char *name;
  int id;
  modules=dyna_create ();
  hive_item_t *item=config_find_item ("Server/Modules");
  if (!item) return 0;
  item=hive_first_child (item);
  while (item)
    {
      name = hive_header_name (item);
      id   = hive_header_int_value (item);
      wt_load_single_module (name, id);

      item = hive_next_sibling (item);
    }
  return 0;
}

void            // Unload testing stuff's modules
wt_unload_modules                  (void)
{
  dyna_destroy (modules, 0);  // Just destroy dynalist
                              // Loaded symbols will be freed when plugins' stuff wll be stopped
}

int             // Send task to module for testing
wt_module_send_for_testing         (wt_task_t *__self, char *__error)
{
	wt_module_t *lib;
  dyna_item_t *item;
  dyna_search_reset (modules);
  item=dyna_search (modules, (void*)ITOL (__self->lid), 0, wt_module_id_comparator);
  if (!item)
    {
      strcpy (__error, "No such module");
      return -1;
    }

	lib=dyna_data (item);
	if (!lib || !lib->procs.StartForTesting)// But why???
    {
      strcpy (__error, "No StartForTesting entrypoint in module");
      return -1;
    }

	if (!lib->procs.StartForTesting (__self,__error))
	  return -1;

  return 0;
}

char*           // Return testing module name
wt_module_name                     (int __lid)
{
	wt_module_t *lib;
  dyna_item_t *item;
  dyna_search_reset (modules);
  item=dyna_search (modules, (void*)ITOL (__lid), 0, wt_module_id_comparator);
  lib=dyna_data (item);
  if (!item || !lib || !lib->name) return "Undefined";
  return lib->name;
}

////////////////////////////////////////
// Plugins' library stuff

static int      // Load one plugin
load_single_plugin                 (char *__lib)
{
  char fname[65536], *full=0;

  // Attach prefix and suffix to libname
  sprintf (fname, "%s%s%s", LIB_PREFIX, __lib, LIB_SUFFIX);
  strlowr (fname, fname);
  full=get_full_path (fname);

  core_print (MSG_INFO, "        Loading plugin %s...", __lib);

  if (plugin_load (full))
    {
      CMSG_FAILED ();
      free (full);
      return -1;
    }
  CMSG_OK ();
  free (full);
  return 0;
}

int             // Load all plugins
wt_load_plugins                    (void)
{
  char *name;

  hive_item_t *item=config_find_item ("Server/Plugins");
  if (!item) return 0;
  item=hive_first_child (item);
  while (item)
    {
      name = hive_header_name (item);
      load_single_plugin (name);

      item = hive_next_sibling (item);
    }
  return 0;
}
