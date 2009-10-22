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


#ifndef _plugin_defs_h_
#define _plugin_defs_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/dynastruc.h>

#define MAX_LOAD_PLUGINS 1024
#define PLUGIN_INIT_PROC "__plugin_init_entry_"

#define PLUGIN_SYM(lib,proc,type,sym) \
  proc=(type)plugin_sym (lib, sym); \
  if (!proc) \
    { \
      if (!strcmp (core_get_last_error (), "")) \
        core_set_last_error ("%s", dlerror ()); \
    }

#define PLUGIN_INIT(_name, initproc, plugininfo) \
  int __plugin_init_entry_ (plugin_t *__self)\
  {\
    __self->info=plugininfo;\
    initproc ((__self));\
    strcpy (__self->name, _name);\
    return plugin_register (__self);\
  }

struct _plugin_t;
typedef struct _plugin_t plugin_t;

typedef int (*plugin_init_proc) (plugin_t *__plugin);

typedef int (*plugin_load_proc) (plugin_t *__plugin);
typedef int (*plugin_unload_proc) (plugin_t *__plugin);

typedef int (*plugin_activate_proc) (plugin_t *__plugin);
typedef int (*plugin_deactivate_proc) (plugin_t *__plugin);

typedef struct
{
  plugin_init_proc Init;
} plugin_procs_t;

typedef struct
{
  /* General info */
  int major_version;
  int minor_version;

  /* Callbacks */
  plugin_load_proc plugin_load;
  plugin_unload_proc plugin_unload;

  plugin_activate_proc plugin_activate;
  plugin_deactivate_proc plugin_deactivate;
} plugin_info_t;

struct _plugin_t
{
  void *dl;
  char fn[128];
  char name[128];
  plugin_procs_t procs;

  plugin_info_t info;

  int activated;
};

#include <libwebtester/plugin.h>

END_HEADER

#endif
