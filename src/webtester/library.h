/**
 * WebTester Server - server of on-line testing system
 *
 * Plugins and testing modules stuff
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef _WT_LIBRARY_H_
#define _WT_LIBRARY_H_

#include <libwebtester/smartinclude.h>

BEGIN_HEADER

#include <webtester/task.h>

#define MODULE_MESSAGE(__type, __module, __text,__args...) \
  core_print (__type, "    " __module ": " __text, ##__args)

#define MODULE_INFO(__module, __text, __args...) \
  MODULE_MESSAGE (MSG_INFO,    __module, __text, ##__args)

#define MODULE_WARNING(__module, __text, __args...) \
  MODULE_MESSAGE (MSG_WARNING, __module, __text, ##__args)

#define MODULE_ERROR(__module, __text, __args...) \
  MODULE_MESSAGE (MSG_ERROR,   __module, __text, ##__args)

#define MODULE_DEBUG(__module, __text, __args...) \
  MODULE_MESSAGE (MSG_DEBUG,   __module, __text, ##__args)


#define MODULE_KEY_ENTRY(__proc, __module, __val, __path) \
  { \
    char buf[4096]; \
    sprintf (buf, "Server/Modules/" __module "/%s", __path); \
    __proc (__val, buf); \
  }

#define MODULE_INT_KEY(__module, __val, __path) \
  MODULE_KEY_ENTRY (CONFIG_INT_KEY, __module, __val, __path)

#define MODULE_FLOAT_KEY(__module, __val, __path) \
  MODULE_KEY_ENTRY (CONFIG_FLOAT_KEY, __module, __val, __path)

#define MODULE_PCHAR_KEY(__module, __val, __path)  \
  MODULE_KEY_ENTRY (CONFIG_PCHAR_KEY, __module, __val, __path)

/****
 *
 */

/* Load modules for testing stuff */
int
wt_load_modules (void);

/* Unload testing stuff's modules */
void
wt_unload_modules (void);

/* Send task to module for testing */
int
wt_module_send_for_testing (wt_task_t *__task, char *__error);

/* Returns name of testing module */
char*
wt_module_name (int __lid);

/* Returns ID of testing module */
int
wt_module_id (const char *__name);

/****
 * Plugins' library stuff
 */

/* Load all plugins */
int
wt_load_plugins (void);

END_HEADER

#endif
