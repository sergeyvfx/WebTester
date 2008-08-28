/*
 *
 * ================================================================================
 *  console-cmd-line.c - part of the Webtester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "console.h"
#include "common.h"

#include <libwebtester/dynastruc.h>

#include <string.h>

#define MAX_COMMANDS 1024

static char *static_cmds[] = {
  "help",
  "bind",
  "varlist",
  "cmdlist",
  0
};


static GtkWidget   *cmd_line=NULL;
static dynastruc_t *cmd_list=NULL;
static dynastruc_t *cmd_hist=NULL;

static dyna_item_t *cmd_cur_item=NULL;

////////
//

static void
preregister_commands               (void)
{
  int i=0;
  while (static_cmds[i])
    {
      console_cmd_line_register (static_cmds[i]);
      i++;
    }
}

static void
cmd_push                           (char *__self)
{
  dyna_push (cmd_hist, strdup (__self), 0);
  cmd_cur_item=0;
}

static char*
cmd_prev                           (void)
{
  if (!dyna_length (cmd_hist))
    return 0;

  if (!cmd_cur_item)
    cmd_cur_item=dyna_head (cmd_hist); else
    if (dyna_next (cmd_cur_item))
      cmd_cur_item=dyna_next (cmd_cur_item);

  return dyna_data (cmd_cur_item);
}

static char*
cmd_next                           (void)
{
  if (!dyna_length (cmd_hist))
    return 0;

  if (!cmd_cur_item)
    cmd_cur_item=dyna_head (cmd_hist); else
    if (dyna_prev (cmd_cur_item))
      cmd_cur_item=dyna_prev (cmd_cur_item);

  return dyna_data (cmd_cur_item);
}

////////////////////////////////////////
// User's backend

int
console_cmd_line_init              (void)
{
  cmd_line=lookup_widget (main_window, "cmd_entry");

  cmd_list=dyna_create ();
  cmd_hist=dyna_create ();

  preregister_commands ();
  return 0;
}

void
console_cmd_line_done              (void)
{
  dyna_destroy (cmd_list, dyna_deleter_free_ref_data);
  dyna_destroy (cmd_hist, dyna_deleter_free_ref_data);
}

void
console_cmd_line_register          (char *__self)
{
  if (!__self)
    return;

  dyna_search_reset (cmd_list);
  if (dyna_search (cmd_list, __self, 0, dyna_string_comparator))
    return;
  dyna_append (cmd_list, strdup (__self), 0);
}

char*
console_cmd_line_autocomplete      (void)
{
  char *cmd=0, *cmds[MAX_COMMANDS];
  const char *prefix;
  int prefix_len, n;

  if (!cmd_line) return 0;

  prefix=gtk_entry_get_text (GTK_ENTRY (cmd_line));
  prefix_len=strlen (prefix);

  n=0;
  DYNA_FOREACH (cmd_list, cmd)
    if (!strncmp (cmd, prefix, prefix_len))
      cmds[n++]=cmd;
  DYNA_DONE;
  
  if (n==1)
    console_cmd_line_set (cmds[0]); else
    {
      char *dummy;
      int i, j;
      
      for (i=0; i<n; i++)
        for (j=0; j<n-1; j++)
          if (strcmp (cmds[j], cmds[j+1])>0)
            {
              dummy=cmds[j];
              cmds[j]=cmds[j+1];
              cmds[j+1]=dummy;
            }

      if (n)
        {
          console_log ("Avaliable autocompletions:\n");
          for (i=0; i<n; i++)
            {
              console_log ("%s ", cmds[i]);
              if (i%3==2)
                console_log ("\n"); else
                console_log ("\t\t\t");
            }
          if ((i-1)%3!=2)
              console_log ("\n");
        }
    }
  
  return cmd;
}

void
console_cmd_line_set               (char *__self)
{
  if (!cmd_line) return;
  
  gtk_entry_set_text (GTK_ENTRY (cmd_line), __self);
  gtk_editable_set_position (GTK_EDITABLE (cmd_line), strlen (__self));
}

char*
console_cmd_line_get               (void)
{
  if (!cmd_line) return "";

  return (char*)gtk_entry_get_text (GTK_ENTRY (cmd_line));
}

void
console_cmd_line_push              (char *__self)
{
  cmd_push (__self);
}

void
console_cmd_line_prev              (void)
{
  char *cmd=cmd_prev ();
  if (cmd)
    console_cmd_line_set (cmd);
}

void
console_cmd_line_next              (void)
{
  char *cmd=cmd_next ();
  if (cmd)
    console_cmd_line_set (dyna_data (cmd_cur_item));
}
