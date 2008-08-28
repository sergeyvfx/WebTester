/*
 *
 * ================================================================================
 *  compiler.c - part of the WebTester Server
 * ================================================================================
 *
 *  Compiler-based stuff.
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "informatics.h"

#include <libwebtester/conf.h>

#include <stdio.h>

static void     // Return full compiler path in config file
compiler_hive_path                 (char *__compiler_id, char *__path)
{
  sprintf (__path, "Server/Compilers/%s", __compiler_id);
}

flex_value_t*   // Parameter in compiler tree
Informatics_compiler_config_val    (char *__compiler_id, char *__path)
{
  flex_value_t *res=NULL;
  char buf[4096], path[4096];
  compiler_hive_path (__compiler_id, buf);
  sprintf (path, "%s/%s", buf, __path);
  CONFIG_OPEN_KEY (res, path);
  return res;
}

flex_value_t*   // Parameter in compiler tree
Informatics_compiler_common_val    (char *__path)
{
  flex_value_t *res=NULL;
  char path[4096];
  sprintf (path, "Server/Compilers/%s", __path);
  CONFIG_OPEN_KEY (res, path);
  return res;
}
