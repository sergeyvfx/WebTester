/*
 *
 * ================================================================================
 *  console.c - part of the WebTester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "console.h"
#include "support.h"
#include "common.h"

#include <libwebtester/macrodef.h>
#include <string.h>

#include <malloc.h>

static int sock = -1;

////
//

static void
log_to_widget_entry                (char *__console, char *__scroll, char *__text)
{
  GtkWidget *console;
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  GtkWidget *sw;
  GtkAdjustment *adj;
  char *dummy=0, *buf, *txt;

  if (!__text || !strlen (__text)) return;
  
  // Append text to console's buffer
  console=lookup_widget (main_window, __console);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (console));

  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);

  dummy=gtk_text_buffer_get_text (buffer, &start, &end, TRUE);

  buf=malloc (strlen (dummy)+strlen (__text)+1);
  strcpy (buf, dummy);
  strcat (buf, __text);

  if (strlen (buf)>4096)
    {
      char *stxt;
      stxt=txt=buf+strlen (buf)-4096;
      while (*stxt && *stxt!='\n')
        stxt++;
      if (stxt)
        txt=stxt;
    } else txt=buf;

  buffer=gtk_text_buffer_new (0);

  gtk_text_buffer_set_text (buffer, txt, strlen (txt));
  gtk_text_view_set_buffer (GTK_TEXT_VIEW (console), buffer);

  free (dummy);
  free (buf);

  // Scroll to the end
  if (GTK_WIDGET_VISIBLE(console))
    {
      sw=lookup_widget (main_window, __scroll);
      adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (sw));
      gtk_adjustment_set_value (adj, adj->upper-0.001);
      gtk_adjustment_value_changed (adj);
      gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (sw), adj);
    }
}

////
//

int
console_init                       (void)
{
  console_cmd_init ();
  console_builtin_init ();
  return 0;
}

void
console_done                       (void)
{
  console_builtin_done ();
  console_cmd_done ();
}

void
log_to_widget                      (char *__console, char *__scroll, char *__text, ...)
{
  char print_buf[65536], dummy[65536];
  int i, len=0, n;

  // Pack args to buffer
  PACK_ARGS (__text, print_buf, 65535);

  for (i=0, n=strlen (print_buf); i<n; i++)
    {
      dummy[len++]=print_buf[i];
      if (print_buf[i]=='\n')
       {
         dummy[len]=0;
         log_to_widget_entry (__console, __scroll, dummy);
         len=0;
       }
    }
}

void
console_set_socket                 (int __socket)
{
  sock=__socket;
}

int
console_get_socket                 (void)
{
  return sock;
}
