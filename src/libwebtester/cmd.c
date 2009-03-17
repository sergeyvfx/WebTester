/**
 * WebTester Server - server of on-line testing system
 *
 * Command line context processor
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "cmd.h"
#include "strlib.h"

#include <malloc.h>
#include <string.h>

#include <glib.h>

/**
 * Iterator of parser
 *
 * @param __self - string to parse
 * @param __token - parsed token
 * @param __line - line number
 * @return new shift'ed string
 */
static char*
cmd_parser_iterator (char *__self, char **__token, int *__line)
{
  int len = 0;
  if (!__self)
    {
      return 0;
    }

  /* Skip spaces */
  while ((*__self) <= 32 || (*__self) >= 127)
    {
      if (*__self == '\n') (*__line)++;
      if (!(*__self))
        {
          return 0;
        }
      __self++;
    }

  /* Read string in double qoutes */
  if (*__self == '"')
    {
      __self++;
      while (__self && *__self && (*__self) != '"')
        {
          if (*__self == '\\')
            {
              __self++;
              if (*__self == 'n') *(*__token + len++) = '\n'; else
              if (*__self == 'r') *(*__token + len++) = '\r'; else
              if (*__self == 't') *(*__token + len++) = '\t'; else
                *(*__token + len++) = *__self;
            }
          else
            {
              *(*__token + len++) = *__self;
            }
          __self++;
        }
      __self++;
    }
  else
    {
      /* read string up to spaces */
      while ((*__self) > 32 && (*__self) < 127)
        {
          if (*__self == '\\')
            {
              __self++;
              if (*__self == 'n') *(*__token + len++) = '\n'; else
              if (*__self == 'r') *(*__token + len++) = '\r'; else
              if (*__self == 't') *(*__token + len++) = '\t'; else
                *(*__token + len++) = *__self;
            }
          else
            {
              *(*__token + len++) = *__self;
            }
          __self++;
        }
    }

  *(*__token + len) = 0;
  if (len == 0)
    {
      return 0;
    }
  return __self;
}

/**
 * Destructor for procedure list
 *
 * @param __item - procedure to destroy
 */
static void
cmd_proclist_destroyer (void *__item)
{
  cmd_proc_t *self = __item;
  free (self->name);
  free (__item);
}

/**
 * Comparator for procedure list
 *
 * @param __left - value from list
 * @param __right - name of procedure to find
 * @return non-zero if __left is searched, zero otherwise
 */
static int
cmd_proclist_comparator (void *__left, void *__right)
{
  cmd_proc_t *self = __left;
  if (!strcmp (self->name, __right))
    {
      return 1;
    }
  return 0;
}

/**
 * Get procedure descriptor by name
 *
 * @param __self - descriptor of context
 * @param __name - name of procedure to get
 * @return descriptor of procedure or NULL if there is no such procedure
 */
cmd_proc_t*
cmd_context_get_proc (cmd_context_t *__self, char *__name)
{
  dyna_item_t *item;
  mutex_lock (__self->mutex);
  dyna_search_reset (__self->proclist);
  item = dyna_search (__self->proclist, __name, 0, cmd_proclist_comparator);
  mutex_unlock (__self->mutex);

  if (!item)
    {
      return NULL;
    }

  return item->data;
}

/********
 * User's backend
 */

/**
 * Parse command line
 *
 * @param __buf - buffer to parse
 * @param __argv - values will be stored here
 * @param __arc - count of arguments will be stored here
 * @return zero on success, non-zero otherwise
 */
int
cmd_parse_buf (const char *__buf, char ***__argv, int *__argc)
{
  int argc = 0;
  int linenum = 0;
  char **argv = 0;
  char *shift = (char*)__buf;
  char *token;
  token = malloc (65535);

  while ((shift = cmd_parser_iterator (shift, &token, &linenum)))
    {
      strarr_append (&argv, token, &argc);
    }

  free (token);

  strarr_append (&argv, 0, &argc);

  (*__argc) = argc - 1;
  (*__argv) = argv;

  return 0;
}

/**
 * Free result of parsing
 *
 * @param __argv - list of arguments
 * @param __argc - count of arguments in list
 */
void
cmd_free_arglist (char **__argv, int __argc)
{
  strarr_free (__argv, __argc);
}

/**
 * Create new command line context
 *
 * @param __name - name of context
 * @return descriptor of new context
 */
cmd_context_t*
cmd_create_context (const char *__name)
{
  cmd_context_t *ptr;
  ptr = malloc (sizeof (cmd_context_t));
  ptr->name = malloc (strlen (__name) + 1);
  strcpy (ptr->name, __name);
  ptr->proclist = dyna_create ();
  ptr->mutex = mutex_create ();
  return ptr;
}

/**
 * Destroy command line context
 *
 * @param __self - context to be destroyed
 * @return zero on success, non-zero otherwise
 */
int
cmd_destroy_context (cmd_context_t *__self)
{
  if (!__self)
    {
      return -1;
    }

  dyna_destroy (__self->proclist, cmd_proclist_destroyer);
  mutex_free (__self->mutex);
  free (__self->name);
  free (__self);

  return 0;
}

/**
 * Register new command in context
 *
 * @param __self - descriptor of context where proc. will be added
 * @param __name - name of procedure
 * @param __entry_point - entry point of handler
 * @return zero on success, non-zero otherwise
 */
int
cmd_context_proc_register (cmd_context_t *__self, const char *__name,
                           cmd_entry_point __entry_point)
{
  cmd_proc_t *ptr;

  if (!__self || !__name)
    {
      return -1;
    }

  ptr = malloc (sizeof (cmd_proc_t));
  ptr->name = malloc (strlen (__name) + 1);
  strcpy (ptr->name, __name);
  ptr->entry_point = __entry_point;
  dyna_push (__self->proclist, ptr, 0);
  return 0;
}

/**
 * Execute command in context
 *
 * @param __self - descriptor of context
 * @param __argv - argument list
 * @param __argc - count of arguments
 * @returm zero on success, non-zero otherwise
 */
int
cmd_context_execute_proc (cmd_context_t *__self, char **__argv, int __argc)
{
  cmd_proc_t *proc;
  proc = cmd_context_get_proc (__self, __argv[0]);

  if (!proc || !proc->entry_point)
    {
      return -1;
    }

  proc->entry_point (__argc, __argv);

  return 0;
}

/**
 * Unregister procedure from context
 *
 * @param __self - context to unregister procedure from
 * @param __name - name of procedure to unregister
 * @return zero on success, non-zero otherwise
 */
int
cmd_context_proc_unregister (cmd_context_t *__self, const char *__name)
{
  dyna_item_t *item;

  mutex_lock (__self->mutex);
  dyna_search_reset (__self->proclist);
  item = dyna_search (__self->proclist, (void*)__name, 0,
                      cmd_proclist_comparator);

  if (item)
    {
      dyna_delete (__self->proclist, item, cmd_proclist_destroyer);
    }

  mutex_unlock (__self->mutex);

  return 0;
}
