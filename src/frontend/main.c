/*
 *
 * ================================================================================
 *  main.c - part of te WebTester server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include <config.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"

#include "builtin.h"
#include "common.h"
#include "mainloop.h"
#include "stat.h"
#include "console.h"
#include "iface.h"
#include "pipe.h"

#include <libwebtester/core.h>

#include <glib.h>


GtkWidget *main_window;

int
main (int __argc, char *__argv[])
{
#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  g_thread_init (NULL);
  if (!g_thread_supported ())
    {
      return -1;
    }

  gtk_set_locale ();
  gtk_init  (&__argc, &__argv);

  parse_args (__argc, __argv);

  core_init ();

  //
  // TODO:
  //  Not flexible enough
  //

  add_pixmap_directory ("/home/webtester/usr/pixmaps/frontend");
  add_pixmap_directory ("./pixmaps");

  main_window=create_main_window ();
  load_config_file ();
  iface_set_widgets_connected (FALSE);

  gtk_widget_show (main_window);

  stat_init ();
  pipe_init ();
  console_init ();
  mainloop_init ();

  mainloop_start ();
 
  gtk_main ();

 return 0;
}

