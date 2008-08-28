/*
 *
 * ================================================================================
 *  mainloop.c - part of the WebTester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#include "mainloop.h"
#include "common.h"
#include "stat.h"

#include <glib.h>

////
//

static gboolean // Main loop
mainloop                           (gpointer __unused);

////////////////////////////////////////
// User's backend

int
mainloop_init                      (void)
{
  return 0;
}

void
mainloop_done                      (void)
{
}

void
mainloop_start                     (void)
{
  gtk_timeout_add (25, mainloop, 0);
}

////////////////////////////////////////
// Internal stuff

static gboolean // Main loop
mainloop                           (gpointer __unused)
{
  stat_update_monitors ();

  return TRUE;
}
