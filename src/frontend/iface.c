/*
 *
 * ================================================================================
 *  iface.c - part of the WebTester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "iface.h"
#include "common.h"

#include <libwebtester/smartinclude.h>

static void
pipe_tab_set_visibility            (int __val)
{
  GtkWidget *widget=lookup_widget (main_window, "console_tabs");
  GtkWidget *page=gtk_notebook_get_nth_page (GTK_NOTEBOOK (widget), 1);
  if (__val)
    {
      gtk_notebook_set_show_tabs (GTK_NOTEBOOK (widget), TRUE);
      gtk_widget_show (page);
    } else {
      gtk_notebook_set_show_tabs (GTK_NOTEBOOK (widget), FALSE);
      gtk_notebook_set_current_page (GTK_NOTEBOOK (widget), 0);
      gtk_widget_hide (page);
    }
}

////////////////////////////////////////
// User's backend

void
iface_pipe_tab_show                (void)
{
  pipe_tab_set_visibility (TRUE);
}

void
iface_pipe_tab_hide                (void)
{
  pipe_tab_set_visibility (FALSE);
}

void
iface_set_widgets_connected        (BOOL __connected)
{
  int i;
  char dummy[1024];

  GtkWidget *login            = lookup_widget (main_window, "login"),
            *password         = lookup_widget (main_window, "password"),
            *login_at_connect = lookup_widget (main_window, "login_at_connect"),
            *connect          = lookup_widget (main_window, "connect");

  BOOL tmp=gtk_toggle_button_get_active ((GtkToggleButton*)login_at_connect) && !__connected;

  gtk_toggle_button_set_active ((GtkToggleButton*)connect, __connected);
  gtk_widget_set_sensitive (login_at_connect, !__connected);

  gtk_widget_set_sensitive (login,     tmp);
  gtk_widget_set_sensitive (password,  tmp);

  for (i=0; i<BUTTONS_COUNT; i++)
    {
      sprintf (dummy, "ctrlButton_%d", i);
      gtk_widget_set_sensitive (lookup_widget (main_window, dummy), __connected);
    }
  
  if (!__connected)
    iface_pipe_tab_hide ();
}
