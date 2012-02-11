/* Copyright (C) 2011, 2012
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

#include <stdio.h>

#include <glib.h>
#include <glib/gi18n.h>
#include "interface_common.h"
#include "main.h"
#include "config_window_handler.h"
#include "players_manipulation.h"
#include "ripper_encoder_manipulation.h"
#include "select_frame_handler.h"
#include "job_control.h"
#include "status_frame_handler.h"
#include "cddb.h"
#include "misc_utils.h"
#include "version.h"

#include "main_window_handler.h"

void mw_config_button_clicked (GtkWidget * widget, gpointer callback_data);
void mw_stop_button_clicked (GtkWidget * widget, gpointer callback_data);
void mw_go_button_clicked (GtkWidget * widget, gpointer callback_data);
void mw_pause_button_clicked (GtkWidget * widget, gpointer callback_data);
void mw_exit_button_clicked (GtkWidget * widget, gpointer callback_data);
static int check_dirs ();


void
mw_config_button_clicked (GtkWidget * widget, gpointer callback_data)
{
  main_window_handler (MW_MODE_CONFIG, 0, NULL);
  config_window_handler (WIDGET_CREATE, (_main_data *) callback_data);
  return;
}

void
mw_scan_button_clicked (GtkWidget * widget, gpointer callback_data)
{
  _main_data *main_data;

  main_data = (_main_data *) callback_data;
  memset (main_data, 0, sizeof (_main_data));

  select_frame_handler (WIDGET_DESTROY, 0, main_data);
  select_frame_handler (CLEAR_ENTRIES, 0, main_data);

  /* check to make sure there is a cd in the drive.. if not dim out
     cddb and go buttons to prevent errors from even happening....
     otherwise make sure they can use the buttons */

  if (!scan_cd (main_data))
    main_window_handler (CD_INDRIVE, NULL, NULL);
  else
    main_window_handler (NO_CD_INDRIVE, NULL, NULL);

  select_frame_handler (WIDGET_CREATE, 0, main_data);
  select_frame_handler (SF_SYNC_SELECT_FRAME, 0, main_data);
  return;
}

void
mw_go_button_clicked (GtkWidget * widget, gpointer callback_data)
{
  if (!check_dirs ())
    {
      return;
    }
  play_cd_wav_mp3 (STOP, CD, NULL);
  play_cd_wav_mp3 (STOP, WAV, NULL);
  play_cd_wav_mp3 (STOP, MP3, NULL);
  job_starter ((_main_data *) callback_data);
  return;
}

void
mw_stop_button_clicked (GtkWidget * widget, gpointer callback_data)
{
  play_cd_wav_mp3 (STOP, CD, NULL);
  play_cd_wav_mp3 (STOP, WAV, NULL);
  play_cd_wav_mp3 (STOP, MP3, NULL);
  return;
}

void
mw_cddb_button_clicked (GtkWidget * widget, gpointer callback_data)
{
  _main_data *main_data;

  main_data = (_main_data *) callback_data;
  cddb_main ((_main_data *) callback_data);
  return;
}

void
mw_exit_button_clicked (GtkWidget * widget, gpointer callback_data)
{
  int mode;

  mode = *(int *) callback_data;

  ripperX_exit (NULL, NULL);
  return;
}

GtkWidget *
main_window_handler (int ops, char *statusbar_msg, _main_data * main_data)
{

  static GtkWidget *main_window = NULL, *main_frame, *statusbar;
  static int saved_mode;
  static int count = 0;

  static struct
  {
    GtkToolItem *tool_item;
    void *func;
    //    gpointer callback_data;
    int arrangement;
    /* If arrangement is TRUE, it will be packed using gtk_pack_start
     * function. Otherwise, it will use gtk_pack_end function */
    char *tooltip;
    char *stock_icon;
  } buttons[] =
  {
    { NULL, mw_config_button_clicked, TRUE,
      N_("Configuration"), GTK_STOCK_PREFERENCES },
    { NULL, mw_scan_button_clicked, TRUE,
      N_("Scan CD"), GTK_STOCK_CDROM },
    { NULL, mw_stop_button_clicked, TRUE,
      N_("Stop playing"), GTK_STOCK_MEDIA_STOP },
    { NULL, mw_cddb_button_clicked, TRUE,
      N_("Get track titles from CDDB server"), GTK_STOCK_NETWORK },
    { NULL, mw_go_button_clicked, TRUE,
      N_("Start ripping&encoding"), GTK_STOCK_EXECUTE },
    { NULL, mw_exit_button_clicked, FALSE,
      N_("Exit the program"), GTK_STOCK_QUIT },
  };

  /*
  buttons[0].callback_data = (gpointer) main_data;
  buttons[1].callback_data = (gpointer) main_data;
  buttons[3].callback_data = (gpointer) main_data;
  buttons[4].callback_data = (gpointer) main_data;
  buttons[5].callback_data = (gpointer) & saved_mode;
  */

  switch (ops)
    {
    case WIDGET_CREATE:
      {
	GtkWidget *vbox;
	GtkWidget *toolbar;
	int pos_button, num_buttons;
	char welcome_msg_buf[100];

	num_buttons = sizeof(buttons) / sizeof(buttons[0]);
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/* main_frame and status_bar are declared at the top of function. */
	main_frame = gtk_frame_new(NULL);
	statusbar = gtk_statusbar_new();
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	toolbar = gtk_toolbar_new();

	/* Next line is from old ripperX sources. It can surely be removed
	   or second argument should be renamed to mainwindow. */
	gtk_widget_set_name(main_window, "main window");
	gtk_window_set_title(GTK_WINDOW(main_window), PACKAGE_NAME);
	gtk_window_set_default_size(GTK_WINDOW(main_window), 600, 400);
	g_signal_connect(G_OBJECT(main_window), "destroy",
			 G_CALLBACK(ripperX_exit), NULL);
	g_signal_connect(G_OBJECT(main_window), "delete_event",
			 G_CALLBACK(ripperX_exit), NULL);
	sprintf(welcome_msg_buf, PACKAGE_STRING);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), 1, welcome_msg_buf);

	/* Read main_data and create select window here. */
	count = scan_cd(main_data);
	select_frame_handler(WIDGET_CREATE, 0, main_data);
	select_frame_handler(SF_SYNC_SELECT_FRAME, 0, main_data);

	for (pos_button = 0; pos_button < num_buttons; pos_button++)
	  {
	    GtkToolItem *tool_item;
	    tool_item = gtk_menu_tool_button_new_from_stock
	      (buttons[pos_button].stock_icon);
	    gtk_widget_set_tooltip_text(GTK_WIDGET(tool_item),
					buttons[pos_button].tooltip);
	    if (pos_button != 5)
	      {
		g_signal_connect(G_OBJECT(tool_item), "clicked",
				 G_CALLBACK(buttons[pos_button].func),
				 main_data);
	      }
	    else
	      {
		g_signal_connect(G_OBJECT(tool_item), "clicked",
				 G_CALLBACK(buttons[pos_button].func),
				 &saved_mode);
	      }

	    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(tool_item), -1);
	    buttons[pos_button].tool_item = tool_item;
	  }
      
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), main_frame, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(main_window), vbox);
	gtk_widget_show_all(main_window);

	if (config.cddb_config.auto_lookup && !count)
	  {
	    cddb_main(main_data);
	  }
      }
    case MW_MODE_SELECT:
      {
	saved_mode = ops;
	/* where now is used by ripperX_exit */
	where_now = SELECT_FRAME;
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[0].tool_item), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[1].tool_item), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[2].tool_item), TRUE);
	/* disable cddb and go buttons since no cd anyway... prevent
	   stupid errors */
	if (count)
	  {
	    gtk_widget_set_sensitive (GTK_WIDGET(buttons[3].tool_item), FALSE);
	    gtk_widget_set_sensitive (GTK_WIDGET(buttons[4].tool_item), FALSE);
	  }
	else
	  {
	    gtk_widget_set_sensitive (GTK_WIDGET(buttons[3].tool_item), TRUE);
	    gtk_widget_set_sensitive (GTK_WIDGET(buttons[4].tool_item), TRUE);
	  }
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[5].tool_item), TRUE);

	return main_frame;
      }
    case MW_MODE_STATUS:
      {
	saved_mode = ops;
	/* where now is used by ripperX_exit */
	where_now = STATUS_FRAME;
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[0].tool_item), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[1].tool_item), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[2].tool_item), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[3].tool_item), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[4].tool_item), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[5].tool_item), TRUE);

	return main_frame;
      }
    case MW_MODE_CONFIG:
      {
	saved_mode = ops;
	/* where now is used by ripperX_exit */
	where_now = CONFIG_WINDOW;
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[0].tool_item), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[1].tool_item), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[2].tool_item), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[3].tool_item), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[4].tool_item), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[5].tool_item), TRUE);

	return main_frame;
      }
    case MW_CLEAR_STATUSBAR:
      {
	while (--count >= 0)
	  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
	count++;
	return main_frame;
      }
    case MW_UPDATE_STATUSBAR:
      {
	count++;
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, statusbar_msg);
	return main_frame;
      }

    case MW_REQUEST_MW:
      {
	return main_window;
      }

    case MW_REQUEST_MF:
      return main_frame;

    case NO_CD_INDRIVE:
      {
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[3].tool_item), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[4].tool_item), FALSE);
	return main_frame;
      }
    case CD_INDRIVE:
      {
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[3].tool_item), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET(buttons[4].tool_item), TRUE);
	return main_frame;
      }
    }
  return main_frame;
}

static int
check_dirs ()
{
  long wav_free, mp3_free;
  int rc;
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  rc = check_dir (config.wav_path);
  switch (rc)
    {
    case MISC_DOES_NOT_EXISTS:
      rc = dialog_handler (WIDGET_CREATE, FALSE, 7, 0, NULL, NULL, 0);
      if (!rc)
	{
	  return 0;
	}
      else
	{
	  if (create_dir (config.wav_path) != 0)
	    {
	      err_handler (GTK_WINDOW(main_window), 29, NULL);
	      return 0;
	    }
	}
      break;
    case MISC_NOT_DIR:
    case MISC_NOT_WRITABLE:
      err_handler (GTK_WINDOW(main_window), 27, NULL);
      return 0;
      break;
    }

  rc = check_dir (config.mp3_path);
  switch (rc)
    {
    case MISC_DOES_NOT_EXISTS:
      rc = dialog_handler (WIDGET_CREATE, FALSE, 8, 0, NULL, NULL, 0);
      if (!rc)
	{
	  return 0;
	}
      else
	{
	  if (create_dir (config.mp3_path) != 0)
	    {
	      err_handler (GTK_WINDOW(main_window), 30, NULL);
	      return 0;
	    }
	}
      break;
    case MISC_NOT_DIR:
    case MISC_NOT_WRITABLE:
      err_handler (GTK_WINDOW(main_window), 28, NULL);
      return 0;
      break;
    }

  wav_free = check_free_space (config.wav_path);
  mp3_free = check_free_space (config.mp3_path);

  if (wav_free < 500 * 1024)
    {
      /* warn if less than 500 MB free on wav partition */
      rc = dialog_handler (WIDGET_CREATE, FALSE, 5, 0, NULL, NULL, 0);
      if (!rc)
	{
	  return 0;
	}
    }
  if (wav_free != mp3_free && mp3_free < 100 * 1024)
    {
      /* only warn if on different partition and less
       * than 100 MB free */
      rc = dialog_handler (WIDGET_CREATE, FALSE, 6, 0, NULL, NULL, 0);
      if (!rc)
	{
	  return 0;
	}
    }

  /* good to go */
  return 1;

}
