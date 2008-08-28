/*
 *
 * ================================================================================
 *  common,h - part of the WebTester Server frontend
 * ================================================================================
 *
 *  Written (by Nazgul) under General Public License.
 *
*/

#ifndef _common_h_
#define _common_h_

#include <gtk/gtk.h>

#include <config.h>

#include "macrodef.h"
#include "support.h"

#include <libwebtester/core.h>

extern GtkWidget *main_window;

#define BUTTONS_COUNT    7

#define CONSOLE_VIEW     "console_view"
#define CONSOLE_SCROLL   "console_scroll"

#define PIPE_VIEW        "pipe_view"
#define PIPE_SCROLL      "pipe_scroll"

#define CONFIG_FILE      HOME_DIRECTORY "/conf/gwebtester.conf"

#endif
