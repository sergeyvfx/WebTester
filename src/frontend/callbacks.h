/*
 *
 * ================================================================================
 *  callbacks.h - part of te WebTester server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _callbacks_h_
#define _callbacks_h_

#include <gtk/gtk.h>

void
on_main_window_remove              (GtkContainer *__container, GtkWidget *__widget, gpointer __user_data);

void
on_btn_cmdSend_clicked             (GtkButton *__button, gpointer __user_data);

gboolean
on_cmd_entry_key_press_event       (GtkWidget *__widget, GdkEventKey *__event, gpointer __user_data);

void
on_login_at_connect_toggled        (GtkToggleButton *__self, gpointer __user_data);

void
on_connect_toggled                 (GtkToggleButton *__self, gpointer __user_data);

void
on_ctrlButton_clicked              (GtkButton *__button, gpointer __user_data);

#endif
