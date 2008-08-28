/*
 *
 * =============================================================================
 *  core-fs.c
 * =============================================================================
 *
 *  Some core built-in stuff
 *
 *  Written (by Nazgul) under GPL
 *
*/

#include "smartinclude.h"
#include "core.h"
#include "dynastruc.h"
#include "flexval.h"
#include "conf.h"

#include <stdio.h>

static dynastruc_t *registeredPaths=0;
static dyna_item_t *currentPathNode=0;

static dyna_item_t*
get_path_entry                     (char *__path)
{
  if (!registeredPaths) return 0;
  dyna_search_reset (registeredPaths);
  return dyna_search (registeredPaths, __path, 0, dyna_string_comparator);
}

////////////////////////////////////////
//

void
core_register_path                 (char *__path)
{
  if (!registeredPaths) registeredPaths=dyna_create ();
  if (get_path_entry (__path)) return;
  dyna_append (registeredPaths, strdup (__path), 0);
}

void
core_register_paths_from_config    (void)
{
  char *path;
  int i, n;
  flex_value_t *paths=0;

  // Get config node
  CONFIG_OPEN_KEY (paths, "CORE/Paths");
  if (!paths) return;

  // Scan array and register paths
  n=FLEXVAL_ARRAY_LENGTH (paths);
  
  for (i=0; i<n; i++)
    {
      path=flexval_get_array_string (paths, i);
      core_register_path (path);
    }
}

////

char*
core_first_registered_path         (void)
{
  currentPathNode=dyna_head (registeredPaths);
  return dyna_data (currentPathNode);
}

char*
core_last_registered_path          (void)
{
  currentPathNode=dyna_tail (registeredPaths);
  return dyna_data (currentPathNode);
}

char*
core_next_registered_path          (void)
{
  currentPathNode=dyna_next (currentPathNode);
  return dyna_data (currentPathNode);
}

char*
core_prev_registered_path          (void)
{
  currentPathNode=dyna_prev (currentPathNode);
  return dyna_data (currentPathNode);
}

dynastruc_t*
core_registered_paths              (void)
{
  return registeredPaths;
}

////

void
core_unregister_paths              (void)
{
  dyna_destroy (registeredPaths, dyna_deleter_free_ref_data);
}
