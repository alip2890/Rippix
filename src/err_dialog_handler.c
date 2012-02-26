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

/* file err_dialog_handler.c
 * 
 * defines procedures for error messages or dialogs
 *
 * Ralf Engels  10/06/1999  changed errno stuff 
 *                          (strange who does such things)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <gtk/gtk.h>

#include "interface_common.h"

#include "err_dialog_handler.h"

void dl_callback (GtkWidget * widget, gpointer callback_data);
/* When with_entry is FALSE and entry_default is not NULL, entry_default
 * will be displayed as extra msg */


const char *err_msg[] = {
  N_("An error has occured while forking"),
  N_("Cannot create pipe"),
  N_("Cannot open /dev/null"),
  N_("Memory allocation error has occurred"),
  N_("An error has occurred while trying to\nreopen a file as a stream (fdopen)"),
  N_("Cannot read from the pipe"),
  N_("Cannot delete file"),
  N_("Cannot open configuration file\nloading default values"),
  N_("Cannot create configuration file ~/.ripperXrc"),
  N_("Cannot open a pty\nMaybe all ptys are busy"),
  N_("/////  use perror msg until here /////"),
  N_("Too many arguments"),
  N_("Job already in progress"),
  N_("Please select a track to rip or encode.\nIf you did select tracks, they may already have been ripped."),
  N_("Make sure an audio disc is in the drive and that you are\neither running ripperX as root, are a member\nof the \"cdrom\" group, or otherwise have appropriate\npermissions to access the CDROM device.\n"),
  N_("The file name is too long"),
  N_("Some entry(s) are empty"),
  N_("An error has occurred while parsing ~/.ripperXrc"),
  N_("Some field(s) are empty. Substituting default value"),
  N_("Invalid selection"),
  N_("Could not connect to CDDB server"),
  N_("Connection refused from CDDB server"),
  N_("CDDB Server Error"),
  N_("CD Not Found in CD Database"),
  N_("Cannot parse the format string"),
  N_("Plugin not present"),
  N_("Error while locking file.\nMake sure the WAV dir and MP3 dir paths are correct and\nyou have permission to write to those directories."),
  N_("The WAV dir is not writable."),
  N_("The MP3 dir is not writable."),
  N_("Cannot create WAV dir"),
  N_("Cannot create MP3 dir"),
  N_("Make sure an audio disc is in the CDROM device."),
  N_("Make sure that you are a member of the the \"cdrom\" group,\nor otherwise have appropriate permissions to access the CDROM device."),
};

const struct
{
  char *title;
  char *msg;
} dialog_data[] =
{
  {
  N_("Abort confirmation"), N_("Do you really want to abort?")},
  {
  N_("Wanna delete?"), N_("Do you want to delete current file?")},
  {
  N_("Wanna overwrite?"),
      N_("The file already exists. Do you want to overwrite?")},
  {
  N_("Enter file name"), N_("Enter new file name")},
  {
  N_("Wanna create config file?"),
      N_
      ("Config file ~/.ripperXrc does not exist.\nDo you want to create config file?")},
  {
  N_("Low diskspace warning"),
      N_
      ("There is less than 500 megabytes free on the wav partition.\n Do you want to continue?")},
  {
  N_("Low diskspace warning"),
      N_
      ("There is less than 100 megabytes free on the mp3 partition.\n Do you want to continue?")},
  {
  N_("Directory does not exist"),
      N_("The WAV dir does not exist. Create it?")},
  {
  N_("Directory does not exist"),
      N_("The MP3 dir does not exist. Create it?")}
};

const char *status_msg[] = {
  N_("Finished ripping"),
  N_("Finished encoding")
};

/* err_handler displays a GtkMessageDialog with an error message.
 */
void
err_handler (GtkWindow *main_window, int err_code, const char *extra_msg)
{
  GtkWidget *error_dialog;

  /* FIXME Display the messages in the window as well. */
  if (err_code < END_PERROR)
    {
      g_warning("%s\n", strerror(errno));
    }
  if (extra_msg != NULL)
    {
      g_warning("%s\n", extra_msg);
    }

  error_dialog = gtk_message_dialog_new
    (main_window, GTK_DIALOG_DESTROY_WITH_PARENT,
     GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error Code %d", err_code);

  gtk_window_set_title (GTK_WINDOW(error_dialog), _("Error"));
  gtk_message_dialog_format_secondary_text
    (GTK_MESSAGE_DIALOG(error_dialog), "%s", _(err_msg[err_code]));
  g_signal_connect_swapped (G_OBJECT(error_dialog), "response",
			    G_CALLBACK (gtk_widget_destroy),
			    G_OBJECT(error_dialog));

  gtk_widget_show_all(error_dialog);

}

void
dl_callback (GtkWidget * widget, gpointer callback_data)
{
  dialog_handler (DL_OK_PRESSED, TRUE, 0, TRUE, NULL, NULL, 0);
}

int
dialog_handler (int ops, int ok_or_yes, int dialog_code,
		int with_entry, char *entry_default,
		char *entered, int answer_length)
{
  static GtkWidget *window, *vbox, *hbox, *label, *entry, *button, *separator;
  static int answer, saved_with_entry;
  static char *saved_entered;
  int id;

  switch (ops)
    {
    case WIDGET_CREATE:
      saved_entered = entered;
      saved_with_entry = with_entry;

      window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
      gtk_window_set_title (GTK_WINDOW (window),
			    dialog_data[dialog_code].title);
      id =
	g_signal_connect (G_OBJECT (window), "destroy",
			  G_CALLBACK (gtk_main_quit), NULL);
      vbox = gtk_vbox_new (FALSE, 3);
      gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
      gtk_container_add (GTK_CONTAINER (window), vbox);

      hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

      label = gtk_label_new (dialog_data[dialog_code].msg);
      gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

      if (with_entry)
	{
	  entry = gtk_entry_new ();
	  gtk_entry_set_max_length (GTK_ENTRY (entry), answer_length);
	  gtk_entry_set_text (GTK_ENTRY (entry), entry_default);
	  gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);
	  gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
	}
      else if (entry_default != NULL)
	{
	  label = gtk_label_new (entry_default);
	  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
	}

      separator = gtk_hseparator_new ();
      gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);

      hbox = gtk_hbox_new (TRUE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

      if (ok_or_yes == TRUE)
	{
	  button = gtk_button_new_from_stock (GTK_STOCK_OK);
	}
      else
	{
	  button = gtk_button_new_from_stock (GTK_STOCK_YES);
	}
      gtk_widget_set_can_default (button, TRUE);
      g_signal_connect (G_OBJECT (button), "clicked",
			G_CALLBACK (dl_callback), NULL);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
      gtk_widget_grab_default (button);

      if (ok_or_yes == TRUE)
	{
	  button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	}
      else
	{
	  button = gtk_button_new_from_stock (GTK_STOCK_NO);
	}
      gtk_widget_set_can_default (button, TRUE);
      g_signal_connect (G_OBJECT (button), "clicked",
			G_CALLBACK (gtk_main_quit), NULL);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

      gtk_widget_show_all (window);
      answer = FALSE;
      gtk_main ();
      g_signal_handler_disconnect (G_OBJECT (window), id);
      gtk_widget_destroy (window);
      return answer;

    case DL_OK_PRESSED:
      if (saved_with_entry == FALSE)
	{
	  answer = TRUE;
	  gtk_main_quit ();
	  return TRUE;
	}
      if (strlen (gtk_entry_get_text (GTK_ENTRY (entry))) == 0)
	return FALSE;
      strcpy (saved_entered, gtk_entry_get_text (GTK_ENTRY (entry)));
      answer = TRUE;
      gtk_main_quit ();
      return TRUE;
    }
  /* Just to avoid compile time warning */
  return FALSE;
}

void
status_handler (int status_code, const char *extra_msg)
{
  GtkWidget *window, *vbox, *label, *separator, *button;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_title (GTK_WINDOW (window), _("Status"));
  gtk_container_set_border_width (GTK_CONTAINER (window), 5);

  vbox = gtk_vbox_new (FALSE, 3);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  label = gtk_label_new (status_msg[status_code]);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, FALSE, 0);

  if (extra_msg != NULL)
    {
      label = gtk_label_new (extra_msg);
      gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
      gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, FALSE, 0);
    }

  separator = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_OK);
  gtk_widget_set_can_default (button, TRUE);
  g_signal_connect_swapped (G_OBJECT (button), "clicked",
			    G_CALLBACK (gtk_widget_destroy),
			    G_OBJECT (window));
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, FALSE, 0);
  gtk_widget_grab_default (button);

  gtk_widget_show_all (window);
}
