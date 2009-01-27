/**
 * WebTester Server - server of on-line testing system
 *
 * Plugins support stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _plugin_h_
#define _plugin_h_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <libwebtester/dynastruc.h>
#include <libwebtester/plugin-defs.h>

/* Register plugin in pool */
int
plugin_register (const plugin_t *__self);

/* Load plugin */
int
plugin_load (char *__fn);

/* Unload plugin */
int
plugin_unload (plugin_t *__self);

/* Unload plugin by it's file name */
int
plugin_unload_with_fn (char *__fn);

/* Activate plugin */
int
plugin_activate (plugin_t *__self);

/* Activate plugin by it's name */
int
plugin_activate_by_name (const char *__name);

/* Activate plugin by it's library file name */
int
plugin_activate_by_fn (const char *__fn);

/* Deactivate plugin */
int
plugin_deactivate (plugin_t *__self);

/* Deactivate plugin by it's name */
int
plugin_deactivate_by_fn (const char *__fn);

/* Deactivate plugin by it's name */
int
plugin_deactivate_by_name (const char *__name);

/* Unload all plugins */
void
plugin_unload_all (void);

/* Search plugin descitpor by plugin name */
plugin_t*
plugin_search (const char *__plugin_name);

/* Get plugins's library symbol */
void*
plugin_sym (const char *__plugin_name, const char *__sym_name);

/* Get last error occured in plugin stuff */
char*
plugin_load_error (void);

END_HEADER

#endif
