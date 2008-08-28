/*
 *
 * ================================================================================
 *  library.h - part of the WebTester Server
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _wt_library_h_
#define _wt_library_h_

#include <libwebtester/smartinclude.h>
#include <webtester/task.h>

#define MODULE_MESSAGE(__type, __module, __text,__args...) \
  core_print (__type, "    " __module ": " __text, ##__args)

#define MODULE_INFO(__module, __text, __args...)    MODULE_MESSAGE (MSG_INFO,    __module, __text, ##__args)
#define MODULE_WARNING(__module, __text, __args...) MODULE_MESSAGE (MSG_WARNING, __module, __text, ##__args)
#define MODULE_ERROR(__module, __text, __args...)   MODULE_MESSAGE (MSG_ERROR,   __module, __text, ##__args)
#define MODULE_DEBUG(__module, __text, __args...)   MODULE_MESSAGE (MSG_DEBUG,   __module, __text, ##__args)


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

int             // Load modules for testing stuff
wt_load_modules                    (void);

void            // Unload testing stuff's modules
wt_unload_modules                  (void);

int             // Send task to module for testing
wt_module_send_for_testing         (wt_task_t *__task, char *__error);

char*           // Returns name of testing module
wt_module_name                     (int __lid);

////////////////////////////////////////
// Plugins' library stuff

int             // Load all plugins
wt_load_plugins                    (void);

#endif
