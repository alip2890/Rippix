/* Copyright (C) 2011
   Tony Mancill <tmancill@users.sourceforge.net>
   Dave Cinege <dcinege@psychosis.com>
   jos.dehaes@bigfoot.com
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>

#include "misc_utils.h"
#include "interface_common.h"

#include "dir_window_handler.h"

void dw_cancel_clicked ();
void dw_ok_clicked ();
void dw_filew_response (GtkDialog * dialog, gint response_id,
			gpointer user_data);

void
dw_filew_response (GtkDialog * dialog, gint response_id, gpointer user_data)
{
  switch (response_id)
    {
    case GTK_RESPONSE_ACCEPT:
      dw_ok_clicked ();
      break;
    case GTK_RESPONSE_CANCEL:
      dw_cancel_clicked ();
      break;
    }
}

void
dw_cancel_clicked ()
{
  dir_window_handler (DW_CANCEL, NULL);
}

void
dw_ok_clicked ()
{
  dir_window_handler (DW_OK, NULL);
}

char *
dir_window_handler (int ops, char *cur_dir)
{
  static GtkWidget *filew;
  static int id;
  static char buf[MAX_FILE_PATH_LENGTH];
  static char *saved_cur_dir;

  switch (ops)
    {
    case WIDGET_CREATE:
      {
	struct stat st;
	char check_dir[MAX_FILE_PATH_LENGTH + 1];
	char *checked_dir;
	saved_cur_dir = cur_dir;
	checked_dir = check_dir;
	strncpy (check_dir, cur_dir, MAX_FILE_PATH_LENGTH);

	/* Add a final / to input directory string if missing and input string is only a directory (i.e. no filename); but pass through any other strings.  This allows
	   GtkFileSelection widget to show proper directory level upon entering. */
	if ((lstat (checked_dir, &st) >= 0) && (S_ISDIR (st.st_mode)))
	  {
	    if ((check_dir[strlen (checked_dir) - 1] != '/')
		&& (strlen (checked_dir) > 0)
		&& (strlen (checked_dir) < MAX_FILE_PATH_LENGTH))
	      {
		checked_dir = strcat (check_dir, "/");
	      }
	  }

	filew =
	  gtk_file_chooser_dialog_new (_("Select a directory"), NULL,
				       GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				       NULL);
	gtk_window_set_position (GTK_WINDOW (filew), GTK_WIN_POS_MOUSE);

	id = g_signal_connect (G_OBJECT (filew), "destroy",
			       G_CALLBACK (dw_cancel_clicked), NULL);

	g_signal_connect (G_OBJECT (filew),
			  "response", G_CALLBACK (dw_filew_response), NULL);
	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (filew), checked_dir);

	gtk_widget_show (filew);
	gtk_main ();
	g_signal_handler_disconnect (G_OBJECT (filew), id);
	strncpy (buf,
		 gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filew)),
		 sizeof (buf));
	gtk_widget_destroy (filew);
	return buf;
      }

    case DW_OK:
      {
	struct stat st;
	char *temp;

	temp = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filew));
	if (lstat (temp, &st) < 0)
	  {
	    err_handler (INVALID_FILE_SELECTION_ERR, NULL);
	    return NULL;
	  }
	if (!S_ISDIR (st.st_mode))
	  {
	    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (filew),
					   file_path_without_name (temp));
	    temp = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filew));
	  }
	/* remove final directory ../ from directory string if present */
	if (strlen (temp) > 3)
	  {
	    if ((temp[strlen (temp) - 1] == '/') &&
		(temp[strlen (temp) - 2] == '.') &&
		(temp[strlen (temp) - 3] == '.') &&
		(temp[strlen (temp) - 4] == '/'))
	      {
		temp[strlen (temp) - 3] = '\0';
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (filew),
					       temp);
	      }
	  }
	/* remove final directory ./ from directory string if present */
	if (strlen (temp) > 2)
	  {
	    if ((temp[strlen (temp) - 1] == '/') &&
		(temp[strlen (temp) - 2] == '.') &&
		(temp[strlen (temp) - 3] == '/'))
	      {
		temp[strlen (temp) - 2] = '\0';
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (filew),
					       temp);
	      }
	  }
	gtk_main_quit ();
	return NULL;
      }

    case DW_CANCEL:
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (filew), saved_cur_dir);
      gtk_main_quit ();
      return NULL;
    }
  return NULL;
}
