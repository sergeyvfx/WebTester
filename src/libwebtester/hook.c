/*
 *
 * =============================================================================
 *  hook.c
 * =============================================================================
 *
 *  Hooks stuff
 *
 *  Written (by Nazgul) under GPL
 *
*/

#include "hook.h"
#include "dynastruc.h"

#include <malloc.h>
#include <string.h>

static dynastruc_t **hooks;

int
hook_init                          (void)
{
  int i;
  if (hooks) return 0;
  hooks=malloc (sizeof (hook_node_t*)*HOOK_MAX_PRIORITY);
  for (i=0; i<HOOK_MAX_PRIORITY; i++)
    hooks[i]=dyna_create ();
  return 0;
}

void
hook_done                          (void)
{
  int i;
  if (!hooks) return;
  for (i=0; i<HOOK_MAX_PRIORITY; i++)
    dyna_destroy (hooks[i], dyna_deleter_free_ref_data);
  free (hooks);
  hooks=0;
}

hook_node_t
hook_spawn_node                    (char *__proc, hook_callback_proc __callback, void *__userData)
{
  hook_node_t ptr=malloc (sizeof (hook_t));
  strncpy (ptr->proc, __proc, 1024);
  ptr->callback=__callback;
  ptr->userData=__userData;
  return ptr;
}

int
hook_register                      (char *__proc, hook_callback_proc __callback, void *__userData, int __priority)
{
  hook_node_t node;
  if (__priority<0 || __priority>=HOOK_MAX_PRIORITY) return -1;
  hook_init ();
  node=hook_spawn_node (__proc, __callback, __userData);
  dyna_append (hooks[__priority], node, 0);
  return 0;
}

int
hook_unregister                    (char *__proc, hook_callback_proc __callback, int __priority)
{
  int res=-1;
	hook_node_t data;
	dyna_item_t *cur, *dummy;
	if (__priority<0 || __priority>=HOOK_MAX_PRIORITY) return -1;

	cur=dyna_head (hooks[__priority]);
	while (cur)
		{
			data=dyna_data (cur);
			if (data->callback==__callback && !strcmp (data->proc, __proc))
				{
					dummy=cur;
					cur=dyna_next (cur);
					dyna_delete (hooks[__priority], dummy, dyna_deleter_free_ref_data);
          res=0;
				} else
					cur=dyna_next (cur);
		}
  return res;
}

int
hook_call                          (char *__proc, void *__data)
{
  int i;
  dyna_item_t *cur;
  hook_node_t data;
  if (!hooks) return 0;
  for (i=HOOK_MAX_PRIORITY-1; i>=0; i--)
    {
      cur=dyna_head (hooks[i]);
      while (cur)
        {
          data=dyna_data (cur);
          if (!strcmp (data->proc, __proc))
            if (data->callback (data->userData, __data))
              return -1;
          cur=dyna_next (cur);
        }
    }
  return 0;
}

int
hook_call_backward                 (char *__proc, void *__data)
{
  int i;
  dyna_item_t *cur;
  hook_node_t data;
  if (!hooks) return 0;
  for (i=HOOK_MAX_PRIORITY-1; i>=0; i--)
    {
      cur=dyna_tail (hooks[i]);
      while (cur)
        {
          data=dyna_data (cur);
          if (!strcmp (data->proc, __proc))
            if (data->callback (data->userData, __data))
              return -1;
          cur=dyna_prev (cur);
        }
    }
  return 0;
}
