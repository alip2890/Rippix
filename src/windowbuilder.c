#define _GNU_SOURCE
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

#include <config.h>
#include <locale.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include "gettext.h"
#include "windowbuilder.h"

/* Contains nodes of struct _windowbuilder_builder to maintain
   the list of displayable windows. */
static GSList *builders;

GtkBuilder*
windowbuilder_new_builder (const gchararray window_name)
{
  GtkBuilder *builder;
  GError *add_error = NULL;
  GString *uifilestr = g_string_new ("");
  int length = g_slist_length (builders);

  /* Check if the builder was already created and return it. */
  if (length != 0)
    {
      builder = windowbuilder_get_builder (window_name);
      if (builder)
	{
	  return builder;
	}
    }

  /* Create the GtkBuilder with appropriate UI file. */
  g_string_printf (uifilestr, "%s%s%s%s", DATAROOTDIR, "/p2pchat/ui/",
		   window_name, ".ui");
  builder = gtk_builder_new ();

  if (!gtk_builder_add_from_file (builder, uifilestr->str, &add_error))
    {
      error (0, 0, _("Unable to load UI file: %s"), add_error->message);
      return NULL;
    }
  gtk_builder_connect_signals (builder, NULL);

  windowbuilder_builder* builder_node = (windowbuilder_builder*) g_malloc
    (sizeof (windowbuilder_builder));
  builder_node->window_name = g_string_new (window_name);
  builder_node->builder = builder;
  builders = g_slist_prepend (builders, (gpointer) builder_node);
  builders = g_slist_reverse (builders);

  return builder;
}

GtkBuilder *
windowbuilder_get_builder (const gchararray window_name)
{
  GString *window_string = g_string_new (window_name);
  guint ssize;

  if (!builders)
    {
      error (0, 0, _("Cannot destroy builder: no builders instantiated"));
      return NULL;
    }

  ssize = g_slist_length (builders);
  for (int pos = 0; pos < ssize; pos++)
    {
      windowbuilder_builder *temp_builder = (windowbuilder_builder*)
	g_slist_nth_data (builders, pos);
      if (g_string_equal (window_string, temp_builder->window_name))
	{
	  return temp_builder->builder;
	}
    }

  return NULL;
}

int
windowbuilder_destroy_builder (const gchararray window_name)
{
  GString *window_string = g_string_new (window_name);
  guint ssize = g_slist_length (builders);

  if (!builders)
    {
      error (0, 0, _("Cannot destroy builder: no builders instantiated"));
      return -1;
    }

  for (int pos = 0; pos < ssize; pos++)
    {
      windowbuilder_builder *temp_builder = (windowbuilder_builder*)
	g_slist_nth_data (builders, pos);
      if (g_string_equal (window_string, temp_builder->window_name))
	{
	  GtkBuilder *builder = temp_builder->builder;
	  GtkWidget *toplevel = GTK_WIDGET
	    (gtk_builder_get_object (builder, "toplevel"));

	  builders = g_slist_remove (builders, temp_builder);
	  g_object_unref (builder);
	  gtk_widget_destroy (toplevel);
	  break;
	}
    }
  return TRUE;
}

int
windowbuilder_show_window (const gchararray window_name)
{
  GtkBuilder *builder;
  GtkWidget *toplevel;

  builder = windowbuilder_get_builder (window_name);
  if (!builder)
    {
      /* No error message because windowbuilder_get_builder already
	 outputs one. */
      return -1;
    }
  toplevel = GTK_WIDGET (gtk_builder_get_object (builder, "toplevel"));
  gtk_widget_show_all (toplevel);
  return TRUE;
}
