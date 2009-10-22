/**
 * WebTester Server - server of on-line testing system
 *
 * Compiler-based stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "informatics.h"

#include <libwebtester/conf.h>

#include <stdio.h>

/**
 * Return full compiler path in config file
 *
 * @param __compier_id - id of compiler to get path to
 */
static void
compiler_hive_path (const char *__compiler_id, char *__path)
{
  sprintf (__path, "Server/Compilers/%s", __compiler_id);
}

/**
 * Parameter in compiler tree
 *
 * @param __compiler_id - id of compiler to get parameter of
 * @param __path - path in config tree
 * @return parameter value
 */
flex_value_t*
Informatics_compiler_config_val (const char *__compiler_id,
                                 const char *__path)
{
  flex_value_t *res = NULL;
  char buf[4096], path[4096];
  compiler_hive_path (__compiler_id, buf);
  snprintf (path, BUF_SIZE (path), "%s/%s", buf, __path);
  CONFIG_OPEN_KEY (res, path);
  return res;
}

/**
 * Parameter in compiler tree
 *
 * @param __path - path in config tree
 * @return parameter value
 */
flex_value_t*
Informatics_compiler_common_val (const char *__path)
{
  flex_value_t *res = NULL;
  char path[4096];
  snprintf (path, BUF_SIZE (path), "Server/Compilers/%s", __path);
  CONFIG_OPEN_KEY (res, path);
  return res;
}
