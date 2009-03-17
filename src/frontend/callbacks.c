/*
 *
 * ================================================================================
 *  callbacks.c - part of te WebTester server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "common.h"
#include "builtin.h"

#include <config.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>

#include "console.h"

void
on_main_window_remove              (GtkContainer *__container, GtkWidget *__widget, gpointer __user_data)
{
  exit (0);
}

void
on_btn_cmdSend_clicked             (GtkButton *__button, gpointer __user_data)
{
  console_cmd_send ();
}

gboolean
on_cmd_entry_key_press_event       (GtkWidget *__widget, GdkEventKey *__event, gpointer __user_data)
{
  switch (__event->keyval)
    {
      case GDK_Escape:
        console_cmd_line_clear ();
        return TRUE;
      case GDK_Tab:
        console_cmd_line_autocomplete ();
        return TRUE;
      case GDK_Return:
        console_cmd_send ();
        return TRUE;
      case GDK_Up:
        console_cmd_line_prev ();
        return TRUE;
      case GDK_Down:
        console_cmd_line_next ();
        return TRUE;
      default:
        return FALSE;
    }
  return FALSE;
}

void
on_login_at_connect_toggled        (GtkToggleButton *__self, gpointer __user_data)
{
  GtkWidget *login    = lookup_widget (main_window, "login"),
            *password = lookup_widget (main_window, "password");
 
  if (gtk_toggle_button_get_active (__self))
    {
      gtk_widget_set_sensitive (login,    TRUE);
      gtk_widget_set_sensitive (password, TRUE);
      gtk_widget_grab_focus (login);
    } else
    {
      gtk_widget_set_sensitive (login,    FALSE);
      gtk_widget_set_sensitive (password, FALSE);
    }
}

void
on_connect_toggled                 (GtkToggleButton *__self, gpointer __user_data)
{
  BOOL auth=gtk_toggle_button_get_active ((GtkToggleButton*)lookup_widget (main_window, "login_at_connect"));
  char *server   = (char*)gtk_entry_get_text ((GtkEntry*)lookup_widget (main_window, "server"));
  char *login    = (char*)gtk_entry_get_text ((GtkEntry*)lookup_widget (main_window, "login"));
  char *password = (char*)gtk_entry_get_text ((GtkEntry*)lookup_widget (main_window, "password"));

  if (gtk_toggle_button_get_active (__self))
    {
      if (!connect_to_server (server, auth, login, password))
        gtk_toggle_button_set_active (__self, FALSE);
    } else {
      disconnect_from_server ();
    }
}

void
on_ctrlButton_clicked              (GtkButton *__button, gpointer __user_data)
{
  int i;
  char name[64];
  for (i=0; i<7; i++)
    {
      snprintf (name, BUF_SIZE (name), "ctrlButton_%d", i);
      if (__button==(GtkButton*)lookup_widget (main_window, name))
        {
          exec_button_command (i);
          break;
        }
    }
}
