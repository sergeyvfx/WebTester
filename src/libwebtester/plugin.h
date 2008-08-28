/*
 *
 * =============================================================================
 *  plugin.h
 * =============================================================================
 *
 *  Plugins support stuff
 *
 *  Written (by Nazgul) under GPL
 *
*/


#ifndef _plugin_h_
#define _plugin_h_

#include <libwebtester/dynastruc.h>
#include <libwebtester/plugin-defs.h>


int
plugin_probe                       (plugin_t *__self);

int
plugin_register                    (plugin_t *__self);


int
plugin_load                        (char *__fn);

int
plugin_unload                      (plugin_t *__self);

int
plugin_unload_with_fn              (char *__fn);

int
plugin_activate                    (plugin_t *__self);

int
plugin_activate_by_name            (char *__name);

int
plugin_activate_by_fn              (char *__fn);

int
plugin_deactivate                  (plugin_t *__self);

int
plugin_deactivate_by_fn            (char *__fn);

int
plugin_deactivate_by_name          (char *__name);


void
plugin_unload_all                  (void);

plugin_t*
plugin_search                      (char *__plugin_name);

void*
plugin_sym                         (char *__plugin_name, char *__sym_name);

char*
plugin_load_error                  (void);

#endif
