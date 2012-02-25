/* Copyright (C) 2012
   Aljosha Papsch <papsch.al@googlemail.com>

   This file is part of Rippix.

   Rippix is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   Rippix is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with Rippix.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef WINDOWBUILDER_H
#define WINDOWBUILDER_H

#define _GNU_SOURCE

#include <config.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <errno.h>

/* This struct is used to find the right GtkBuilder instance in
   windowbuilder_get_builder function. */
typedef struct _windowbuilder_builder windowbuilder_builder;
struct _windowbuilder_builder
{
  const GString *window_name;
  GtkBuilder *builder;
};

/* Creates a new window but does not display it. The correct window is
   determined by window_name, that is the filename of the UI file without
   suffix .ui. This function calls gtk_builder_connect_signals, you have
   to link your application with gmodule-2.0 libraries.
   Returns the newly created GtkBuilder or the existing instance of a
   GtkBuilder if it was allocated before.
 */
GtkBuilder *
windowbuilder_new_builder (const gchararray window_name);

/* Returns the GtkBuilder for the specified window in window_name, that is
   the filename of the UI flie without the suffix .ui. If the GtkBuilder
   cannot be found, NULL is returned.
 */
GtkBuilder *
windowbuilder_get_builder (const gchararray window_name);

/* Removes the GtkBuilder from the window list and destroys the toplevel
   window. The toplevel window must be named 'toplevel'. */
int
windowbuilder_destroy_builder (const gchararray window_name);

int
windowbuilder_show_window (const gchararray window_name);

#endif // WINDOWBUILDER_H
