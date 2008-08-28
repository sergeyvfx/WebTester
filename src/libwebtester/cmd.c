/*
 *
 * ================================================================================
 *  cmd.c
 * ================================================================================
 *
 *  Command line context processor
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "cmd.h"
#include "smartinclude.h"
#include "strlib.h"

#include <malloc.h>
#include <string.h>

#include <glib.h>

char*
cmd_parser_iterator                (char *__self, char **__token, int *__line)
{
  int len=0;
  if (!__self) return 0;
  while ((*__self)<=32 || (*__self)>=127)
    {
      if (*__self=='\n') (*__line)++;
      if (!(*__self)) return 0;
      __self++;
    }
  
  if (*__self=='"')
    {
      __self++;
      while (__self && *__self && (*__self)!='"')
        {
          if (*__self=='\\')
            {
              __self++;
              if (*__self=='n')  *(*__token+len++)='\n'; else
              if (*__self=='r')  *(*__token+len++)='\r'; else
              if (*__self=='t')  *(*__token+len++)='\t'; else
                *(*__token+len++)=*__self;
            } else
              *(*__token+len++)=*__self;
          __self++;
        }
      __self++;
    } else
  while ((*__self)>32 && (*__self)<127)
    {
      if (*__self=='\\')
        {
          __self++;
          if (*__self=='n')  *(*__token+len++)='\n'; else
          if (*__self=='r')  *(*__token+len++)='\r'; else
          if (*__self=='t')  *(*__token+len++)='\t'; else
          *(*__token+len++)=*__self;
        } else
          *(*__token+len++)=*__self;
      __self++;
    }
  *(*__token+len)=0;
  if (len==0) return 0;
  return __self;
}

int
cmd_parse_buf                      (char *__buf, char ***__argv, int *__argc)
{
  int argc=0;
  int linenum=0;
  char **argv=0;
  char *shift=__buf;
  char *token;
  token=malloc (65535);
  while ((shift=cmd_parser_iterator (shift, &token, &linenum)))
    {
      strarr_append (&argv, token, &argc);
    }
  free (token);
  strarr_append (&argv, 0, &argc);
  (*__argc)=argc-1;
  (*__argv)=argv;
  return 0;
}

void
cmd_free_arglist                   (char **__argv, int __argc)
{
  strarr_free (__argv, __argc);
}

void
cmd_proclist_destroyer             (void *__item)
{
  cmd_proc_t *self=__item;
  free (self->name);
  free (__item);
}

int
cmd_proclist_comparator            (void *__left, void *__right)
{
  cmd_proc_t *self=__left;
  if (!strcmp (self->name, __right)) return 1;
  return 0;
}

cmd_context_t*
cmd_create_context                 (char *__name)
{
  cmd_context_t *ptr;
  ptr=malloc (sizeof (cmd_context_t));
  ptr->name=malloc (strlen (__name)+1);
  strcpy (ptr->name, __name);
  ptr->proclist=dyna_create ();
  ptr->mutex=mutex_create ();
  return ptr;
}

int
cmd_destroy_context                (cmd_context_t *__self)
{
  if (!__self) return 0;
  dyna_destroy (__self->proclist, cmd_proclist_destroyer);
  mutex_free (__self->mutex);
  free (__self->name);
  free (__self);
  return 0;
}

int
cmd_context_proc_register          (cmd_context_t *__self, char *__name, cmd_entry_point __entry_point)
{
  cmd_proc_t *ptr;
  ptr=malloc (sizeof (cmd_proc_t));
  ptr->name=malloc (strlen (__name)+1);
  strcpy (ptr->name, __name);
  ptr->entry_point=__entry_point;
  dyna_push (__self->proclist, ptr, 0);
  return 0;
}

cmd_proc_t*
cmd_context_get_proc               (cmd_context_t *__self, char *__name)
{
  dyna_item_t *item;
  mutex_lock (__self->mutex);
  dyna_search_reset (__self->proclist);
  item=dyna_search (__self->proclist, __name, 0, cmd_proclist_comparator);
  mutex_unlock (__self->mutex);
  if (!item) return 0;
  return item->data;
  return 0;
}

int
cmd_context_execute_proc           (cmd_context_t *__self, char **__argv, int __argc)
{
  cmd_proc_t *proc;
  proc=cmd_context_get_proc (__self, __argv[0]);

  if (!proc) return -1;
  if (!proc->entry_point) return -1;

  proc->entry_point (__argc, __argv);

  return 0;
}
