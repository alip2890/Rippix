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
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <gtk/gtk.h>
#include "interface_common.h"
#include "main_window_handler.h"
#include "select_frame_handler.h"
#include "dir_window_handler.h"
#include "misc_utils.h"
#include "rw_config.h"

#include "config_window_handler.h"

#define DEFAULT_BIT_RATE            4

#define CW_OK                       100
#define CW_CANCEL                   101

void cw_g_path_clicked (GtkWidget * widget, gpointer callback_data);
int cw_general_handler (int ops, _main_data * main_data,
			GtkWidget * notebook);
int cw_wav_handler (int ops, _main_data * main_data, GtkWidget * notebook);
int cw_mp3_handler (int ops, _main_data * main_data, GtkWidget * notebook);
int cw_players_handler (int ops, _main_data * main_data,
			GtkWidget * notebook);
int cw_cddb_handler (int ops, _main_data * main_data, GtkWidget * notebook);

void cw_ok_button_clicked (GtkWidget * widget, gpointer callback_data);
void cw_cancel_button_clicked (GtkWidget * widget, gpointer callback_data);



void
cw_g_path_clicked (GtkWidget * widget, gpointer callback_data)
{
  GtkWidget *entry;
  char *temp;

  entry = (GtkWidget *) callback_data;
  gtk_widget_set_sensitive (widget, FALSE);
  /* FIXME: config_window_handler.c warning: passing argument 2 of 'dir_window_handler' discards qualifiers from pointer target type */
  if ((temp =
       dir_window_handler (WIDGET_CREATE,
			   gtk_entry_get_text (GTK_ENTRY (entry)))) != NULL)
    gtk_entry_set_text (GTK_ENTRY (entry), temp);
  gtk_widget_set_sensitive (widget, TRUE);
}

int
cw_general_handler (int ops, _main_data * main_data, GtkWidget * notebook)
{
  static GtkWidget *main_frame;
  static GtkWidget *wav_path_entry, *mp3_path_entry;
  static GtkWidget *wav_file_name_format_entry, *mp3_file_name_format_entry;
  static GtkWidget *prepend_char_entry;
  static GtkWidget *make_mp3_from_existing_wav_check_button;
  static GtkWidget *ask_when_file_exists_check_button;
  static GtkWidget *keep_wav_check_button;
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);
  char buf[2];

  switch (ops)
    {
    case WIDGET_CREATE:
      {
	GtkWidget *frame, *vbox, *table, *hbox, *label, *label2, *button;

	main_frame = gtk_frame_new (_("General Configuration"));
	gtk_container_set_border_width (GTK_CONTAINER (main_frame), 10);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (main_frame), vbox);

	/* Frame for wav */
	frame = gtk_frame_new (_("Wav file"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

	table = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);
	gtk_container_add (GTK_CONTAINER (frame), table);

	label = gtk_label_new (_("File name format: "));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			  0, GTK_EXPAND | GTK_FILL, 0, 0);

	wav_file_name_format_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (wav_file_name_format_entry),
				  MAX_FILE_NAME_LENGTH - 3);
	gtk_entry_set_text (GTK_ENTRY (wav_file_name_format_entry),
			    config.wav_file_name_format);
	gtk_table_attach (GTK_TABLE (table),
			  wav_file_name_format_entry,
			  1, 2, 0, 1,
			  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

	/* Create entry first to give its address as callback data */
	wav_path_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (wav_path_entry),
				  MAX_FILE_PATH_LENGTH - 3);
	button = gtk_button_new_with_label (_("Target Directory: "));
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (cw_g_path_clicked), wav_path_entry);
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			  0, GTK_EXPAND | GTK_FILL, 2, 2);

	gtk_entry_set_text (GTK_ENTRY (wav_path_entry), config.wav_path);
	gtk_table_attach (GTK_TABLE (table),
			  wav_path_entry,
			  1, 2, 1, 2,
			  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

	/* Frame for mp3 */
	frame = gtk_frame_new (_("MP3 file"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

	table = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);
	gtk_container_add (GTK_CONTAINER (frame), table);

	label = gtk_label_new (_("File name format: "));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			  0, GTK_EXPAND | GTK_FILL, 0, 0);

	mp3_file_name_format_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (mp3_file_name_format_entry),
				  MAX_FILE_NAME_LENGTH - 3);
	gtk_entry_set_text (GTK_ENTRY (mp3_file_name_format_entry),
			    config.mp3_file_name_format);
	gtk_table_attach (GTK_TABLE (table), mp3_file_name_format_entry, 1, 2,
			  0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			  0, 0);

	mp3_path_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (mp3_path_entry),
				  MAX_FILE_PATH_LENGTH - 3);

	button = gtk_button_new_with_label (_("Target Directory: "));
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (cw_g_path_clicked), mp3_path_entry);
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
			  0, GTK_EXPAND | GTK_FILL, 2, 2);

	gtk_entry_set_text (GTK_ENTRY (mp3_path_entry), config.mp3_path);
	gtk_table_attach (GTK_TABLE (table),
			  mp3_path_entry,
			  1, 2, 1, 2,
			  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

	/* Misc options */
	hbox = gtk_hbox_new (FALSE, 3);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new (_("Prepend character : "));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	buf[0] = config.prepend_char;
	buf[1] = '\0';
	prepend_char_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (prepend_char_entry), 1);
	gtk_widget_set_size_request (prepend_char_entry, 16, 0);
	gtk_entry_set_text (GTK_ENTRY (prepend_char_entry), buf);
	gtk_box_pack_start (GTK_BOX (hbox), prepend_char_entry, FALSE, FALSE,
			    0);

	make_mp3_from_existing_wav_check_button
	  =
	  gtk_check_button_new_with_label (_
					   ("Make Mp3 from existing Wav file"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (make_mp3_from_existing_wav_check_button),
				      config.make_mp3_from_existing_wav);
	gtk_box_pack_start (GTK_BOX (vbox),
			    make_mp3_from_existing_wav_check_button, FALSE,
			    FALSE, 0);

	ask_when_file_exists_check_button
	  =
	  gtk_check_button_new_with_label (_
					   ("Ask user when specified file exists"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (ask_when_file_exists_check_button),
				      config.ask_when_file_exists);
	gtk_box_pack_start (GTK_BOX (vbox), ask_when_file_exists_check_button,
			    FALSE, FALSE, 0);

	keep_wav_check_button
	  = gtk_check_button_new_with_label (_("Keep wav files"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (keep_wav_check_button),
				      config.keep_wav);
	gtk_box_pack_start (GTK_BOX (vbox), keep_wav_check_button, FALSE,
			    FALSE, 0);

	label = gtk_label_new (_("General"));
	label2 = gtk_label_new (_("General"));
	gtk_notebook_append_page_menu (GTK_NOTEBOOK (notebook),
				       main_frame, label, label2);
	gtk_widget_show_all (main_frame);
	return 0;
      }

    case CW_OK:
      /* Wav */
      if (strlen (gtk_entry_get_text (GTK_ENTRY (wav_file_name_format_entry)))
	  == 0
	  || strlen (gtk_entry_get_text (GTK_ENTRY (wav_path_entry))) == 0
	  ||
	  strlen (gtk_entry_get_text (GTK_ENTRY (mp3_file_name_format_entry)))
	  == 0
	  || strlen (gtk_entry_get_text (GTK_ENTRY (mp3_path_entry))) == 0
	  || strlen (gtk_entry_get_text (GTK_ENTRY (prepend_char_entry))) ==
	  0)
	{
	  err_handler (GTK_WINDOW(main_window), EMPTY_ENTRY_ERR,
		       _("You need to fill every entry in general page"));
	  return -1;
	}
      strcpy (config.wav_file_name_format,
	      gtk_entry_get_text (GTK_ENTRY (wav_file_name_format_entry)));
      strcpy (config.wav_path,
	      gtk_entry_get_text (GTK_ENTRY (wav_path_entry)));
      strcpy (config.mp3_file_name_format,
	      gtk_entry_get_text (GTK_ENTRY (mp3_file_name_format_entry)));
      strcpy (config.mp3_path,
	      gtk_entry_get_text (GTK_ENTRY (mp3_path_entry)));
      strcpy (buf, gtk_entry_get_text (GTK_ENTRY (prepend_char_entry)));
      config.prepend_char = buf[0];

      config.make_mp3_from_existing_wav =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
				      (make_mp3_from_existing_wav_check_button));
      config.ask_when_file_exists =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
				      (ask_when_file_exists_check_button));
      config.auto_append_extension = TRUE;
      config.keep_wav =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
				      (keep_wav_check_button));

      return 0;
    }
  /* Just to avoid warning */
  return 0;
}

int
cw_wav_handler (int ops, _main_data * main_data, GtkWidget * notebook)
{
  static GtkWidget *main_frame, *ripper_menu, *menu;
  static GtkWidget *extra_options_entry;
  static int num_buttons, option_length, num_plugins;
  static int extra_options_offset, options_offset;
  static struct
  {
    GtkWidget *button;
    char *arg;
    char *text;
  } button[] =
  {
    {
    NULL, "-s ", N_("Force search for drive (ignore /dev/cdrom)")},
    {
    NULL, "-Z ", N_("Disable paranoia (will act like cdda)")},
    {
    NULL, "-Y ", N_("Disable extra paranoia")},
    {
    NULL, "-X ", N_("Disable scratch detection")},
    {
  NULL, "-W ", N_("Disable scratch repair")},};
  static struct
  {
    GtkWidget *menu_item;
    char *ripper;
    char *plugin;
    char *description;
  } plugins[] =
  {
    {
    NULL, "cdparanoia", "ripperX_plugin-cdparanoia", "cdparanoia III"}
  };

  switch (ops)
    {
    case WIDGET_CREATE:
      {
	GtkWidget *vbox, *hbox, *label, *label2;
	int i, temp;

	main_frame = gtk_frame_new (_("Wav Configuration"));
	gtk_container_set_border_width (GTK_CONTAINER (main_frame), 10);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add (GTK_CONTAINER (main_frame), vbox);

	num_buttons = sizeof (button) / sizeof (button[0]);
	num_plugins = sizeof (plugins) / sizeof (plugins[0]);
	options_offset = 11;
	option_length = strlen (button[0].arg);
	extra_options_offset = options_offset + option_length * num_buttons;

	/* Plugin selector menu */
	label = gtk_label_new (_("Ripper plugin"));
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	menu = gtk_combo_box_text_new ();
	ripper_menu = gtk_menu_new ();
	for (i = 0; i < num_plugins; i++)
	  {
	    plugins[i].menu_item =
	      gtk_menu_item_new_with_label (plugins[i].description);
	    gtk_menu_shell_append (GTK_MENU_SHELL (ripper_menu),
				   plugins[i].menu_item);
	    gtk_combo_box_text_prepend (GTK_COMBO_BOX_TEXT (menu), NULL,
					plugins[i].description);
	  }

	gtk_combo_box_set_active(GTK_COMBO_BOX(menu), 0);

	/* set menu to current plugin */
	for (i = 0; i < num_plugins; i++)
	  {
	    if (!strcmp (config.encoder.plugin, plugins[i].plugin))
	      {
		gtk_menu_set_active (GTK_MENU (ripper_menu), i);
	      }
	  }

	/* GTK3 Note: GtkOptionMenu is deprecated.
	   gtk_option_menu_set_menu( GTK_OPTION_MENU( menu ), ripper_menu );
	 */
	gtk_box_pack_start (GTK_BOX (vbox), menu, FALSE, FALSE, 0);

	/* options */
	for (i = 0; i < num_buttons; i++)
	  {
	    temp =
	      config.ripper.ripper[options_offset + option_length * i + 1] ==
	      button[i].arg[1] ? TRUE : FALSE;
	    button[i].button =
	      gtk_check_button_new_with_label (button[i].text);
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					  (button[i].button), temp);
	    gtk_box_pack_start (GTK_BOX (vbox), button[i].button, FALSE,
				FALSE, 0);
	  }

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new (_("Extra Options: "));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	extra_options_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (extra_options_entry),
				  MAX_OPTIONS_LENGTH - 20);
	gtk_entry_set_text (GTK_ENTRY (extra_options_entry),
			    config.ripper.ripper + extra_options_offset);
	gtk_box_pack_start (GTK_BOX (hbox), extra_options_entry, FALSE, FALSE,
			    0);

	label = gtk_label_new (" Wav ");
	label2 = gtk_label_new (" Wav ");
	gtk_notebook_append_page_menu (GTK_NOTEBOOK (notebook),
				       main_frame, label, label2);
	gtk_widget_show_all (main_frame);
	return 0;
      }

    case CW_OK:
      {
	int i, temp;
	GtkWidget *item;

	memset (config.ripper.ripper, 0, sizeof (config.ripper.ripper));

	item = gtk_menu_get_active (GTK_MENU (ripper_menu));
	temp = 0;
	for (i = 0; i < num_plugins; i++)
	  {
	    if (item == plugins[i].menu_item)
	      temp = i;
	  }

	memset (config.ripper.ripper, ' ', options_offset);
	strcpy (config.ripper.ripper, plugins[temp].ripper);
	config.ripper.ripper[strlen (plugins[temp].ripper)] = ' ';

	strcpy (config.ripper.plugin, plugins[temp].plugin);

	for (i = 0; i < num_buttons; i++)
	  {
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (button[i].button)))
	      sprintf (config.ripper.ripper + options_offset +
		       option_length * i, "%s", button[i].arg);
	    else
	      sprintf (config.ripper.ripper + options_offset +
		       option_length * i, "   ");
	  }
	strcpy (config.ripper.ripper + extra_options_offset,
		gtk_entry_get_text (GTK_ENTRY (extra_options_entry)));
	return 0;
      }
    }
  /* Just to avoid warning */
  return 0;
}

int
cw_mp3_handler (int ops, _main_data * main_data, GtkWidget * notebook)
{
  static GtkWidget *main_frame;
  static GtkWidget *extra_options_entry, *encoder_menu, *menu;
  static int num_bitrates, num_plugins;
  static GtkWidget *use_varbitrate_ckbx, *use_high_qual_ckbx, *use_crc_ckbx;
  /* Docs recommend to not use GtkAdjustment in new code (though it's not deprecated). The padding properties of GtkWidget can be used instead. */
  static GtkAdjustment *vbr_qual_adj;
  static GtkAdjustment *pri_adj;
  static GtkWidget *vbr_qual_scale;
  static GtkWidget *pri_scale;

  static struct
  {
    GtkWidget *menu_item;
    int encoding_type;
    char *encoder;
    char *plugin;
    char *description;
    char *bitrate_op;
    char *varbitrate_op;
    char *vbr_qual_op;
    char *high_qual_op;
    char *crc_op;
  } plugins[] =
  {
    {
    NULL, MP2, "toolame", "ripperX_plugin-toolame",
	"Toolame layer 2 encoder", "-b", "", "-v", "", "-e"},
    {
    NULL, MP3, "encode ", "ripperX_plugin-encode", "ISO Encoder v2", "-b",
	"", "", "", "-e"},
    {
    NULL, MP3, "8hz-mp3 ", "ripperX_plugin-8hz-mp3", "8hz-mp3 Encoder",
	"-b", "", "", "", ""},
    {
    NULL, MP3, "lame", "ripperX_plugin-lame", "Lame MP3 Encoder", "-b",
	"--nohist -v", "-V", "-h", "-p"},
    {
    NULL, MP3, "gogo", "ripperX_plugin-gogo", "GoGo MP3 Encoder", "-b",
	"-v 4", "", "", ""},
    {
    NULL, MP3, "bladeenc", "ripperX_plugin-bladeenc",
	"BladeEnc MP3 Encoder", "-br", "", "", "", "-crc"},
    {
    NULL, MP3, "xingmp3enc", "ripperX_plugin-xingmp3enc",
	"Xingmp3enc MP3 Encoder", "-B", "", "", "", ""},
    {
    NULL, MP3, "l3enc", "ripperX_plugin-l3enc",
	"FHG MP3 Encoder (l3enc v2.72)", "-br", "", "", "-hq", "-crc"},
    {
    NULL, MP3, "mp3enc", "ripperX_plugin-mp3enc",
	"FHG MP3 Encoder (mp3enc 3.1)", "-br", "", "", "-qual 9", "-crc"},
    {
    NULL, OGG, "oggenc", "ripperX_plugin-oggenc", "OggVorbis encoder", "-b",
	"", "", "", ""},
    {
    NULL, FLAC, "flac", "ripperX_plugin-flac", "FLAC encoder", "", "", "",
	"", ""},
    {
    NULL, MUSE, "mppenc", "ripperX_plugin-musepack", "Musepack Encoder", "",
	"", "--quality 5", "--quality 8", ""}
  };

  static struct
  {
    GtkWidget *button;
    int bitrate;
  } bitrate[] =
  {
    {
    NULL, 32},
    {
    NULL, 40},
    {
    NULL, 48},
    {
    NULL, 56},
    {
    NULL, 64},
    {
    NULL, 80},
    {
    NULL, 96},
    {
    NULL, 112},
    {
    NULL, 128},			/* Default */
    {
    NULL, 160},
    {
    NULL, 192},
    {
    NULL, 224},
    {
    NULL, 256},
    {
    NULL, 320}
  };

  switch (ops)
    {
    case WIDGET_CREATE:
      {
	GtkWidget *frame, *table, *vbox, *vbox2, *vbox3, *hbox, *label,
	  *label2;
	int i, match;
	char buf[5];

	main_frame = gtk_frame_new (_("Mp3 Configuration"));
	gtk_container_set_border_width (GTK_CONTAINER (main_frame), 10);

	num_bitrates = sizeof (bitrate) / sizeof (bitrate[0]);
	num_plugins = sizeof (plugins) / sizeof (plugins[0]);

	vbox = gtk_vbox_new (FALSE, 3);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_container_add (GTK_CONTAINER (main_frame), vbox);

	/* Plugin selector menu */
	label = gtk_label_new (_("Encoder plugin"));
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
	menu = gtk_combo_box_text_new ();
	encoder_menu = gtk_menu_new ();
	for (i = 0; i < num_plugins; i++)
	  {
	    plugins[i].menu_item =
	      gtk_menu_item_new_with_label (plugins[i].description);
	    gtk_menu_shell_append (GTK_MENU_SHELL (encoder_menu),
				   plugins[i].menu_item);
	    if (!is_found (plugins[i].encoder))
	      {
		gtk_widget_set_sensitive (GTK_WIDGET (plugins[i].menu_item),
					  0);
	      }
	    /* GTK3 Note: GtkComboBoxText doesn't provide setting sensitive option, so an entry gets only added if it's not sensitive.
	       This could be better done with GtkComboBox instead GtkComboBoxText, but for now it is easier. */
	    else
	      {
		gtk_combo_box_text_prepend (GTK_COMBO_BOX_TEXT (menu), NULL,
					    plugins[i].description);
	      }
	  }
	gtk_combo_box_set_active(GTK_COMBO_BOX(menu), 0);

	/* set menu to current plugin */
	for (i = 0; i < num_plugins; i++)
	  if (!strcmp (config.encoder.plugin, plugins[i].plugin))
	    gtk_menu_set_active (GTK_MENU (encoder_menu), i);

	gtk_box_pack_start (GTK_BOX (vbox), menu, TRUE, TRUE, 0);

	/* Frame for bitrate */
	frame = gtk_frame_new (_("BitRate, Default 128kbits"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

	table =
	  gtk_table_new (3, (gint) ceil ((float) num_bitrates / 3), FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 0);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);
	gtk_container_add (GTK_CONTAINER (frame), table);

	/* First radio button */
	snprintf (buf, sizeof (buf), "%3dk", bitrate[0].bitrate);
	bitrate[0].button = gtk_radio_button_new_with_label (NULL, buf);
	gtk_widget_set_size_request (bitrate[0].button, 0, 16);
	gtk_table_attach_defaults (GTK_TABLE (table), bitrate[0].button, 0, 1,
				   0, 1);

	/* And the rest */
	for (i = 1; i < num_bitrates; i++)
	  {
	    snprintf (buf, sizeof (buf), "%3dk", bitrate[i].bitrate);
	    bitrate[i].button =
	      gtk_radio_button_new_with_label (gtk_radio_button_get_group
					       (GTK_RADIO_BUTTON
						(bitrate[0].button)), buf);
	    gtk_widget_set_size_request (bitrate[i].button, 0, 16);
	    gtk_table_attach_defaults (GTK_TABLE (table),
				       bitrate[i].button, i % 3, i % 3 + 1,
				       i / 3, i / 3 + 1);
	  }

	/* Find current bitrate */
	for (i = 0, match = -1; i < num_bitrates && match == -1; i++)
	  {
	    if (config.encoder.bitrate == bitrate[i].bitrate)
	      match = i;
	  }
	if (match == -1)
	  match = DEFAULT_BIT_RATE;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (bitrate[match].button), TRUE);

	/* Var Bitrate */
	use_varbitrate_ckbx =
	  gtk_check_button_new_with_label (_("Use variable bitrate (VBR)"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (use_varbitrate_ckbx),
				      config.encoder.use_varbitrate);
	gtk_container_add (GTK_CONTAINER (vbox), use_varbitrate_ckbx);

	/* VBR quality */
	frame = gtk_frame_new (_("VBR quality (if available)"));
	gtk_container_add (GTK_CONTAINER (vbox), frame);
	vbox2 = gtk_vbox_new (FALSE, 3);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);
	gtk_container_add (GTK_CONTAINER (frame), vbox2);

	vbr_qual_adj =
	  gtk_adjustment_new ((gfloat) CLAMP (config.encoder.vbr_qual, 0, 9),
			      0.0, 9.0, 1.0, 1.0, 0.0);
	vbr_qual_scale = gtk_hscale_new (GTK_ADJUSTMENT (vbr_qual_adj));
	gtk_scale_set_digits (GTK_SCALE (vbr_qual_scale), 0);
	gtk_container_add (GTK_CONTAINER (vbox2), vbr_qual_scale);
	label =
	  gtk_label_new (_
			 ("0=high quality and bigger files, 9=smaller files"));
	gtk_container_add (GTK_CONTAINER (vbox2), label);

	/* encoder priority */
	frame = gtk_frame_new (_("Encoder Priority"));
	gtk_container_add (GTK_CONTAINER (vbox), frame);
	vbox3 = gtk_vbox_new (FALSE, 3);
	gtk_container_set_border_width (GTK_CONTAINER (vbox3), 5);
	gtk_container_add (GTK_CONTAINER (frame), vbox3);

	pri_adj =
	  gtk_adjustment_new ((gfloat)
			      CLAMP (config.encoder.priority, MAX_NICE_LEVEL,
				     MIN_NICE_LEVEL), 0.0, 19.0, 1.0, 1.0,
			      0.0);
	pri_scale = gtk_hscale_new (GTK_ADJUSTMENT (pri_adj));
	gtk_scale_set_digits (GTK_SCALE (pri_scale), 0);
	gtk_container_add (GTK_CONTAINER (vbox3), pri_scale);
	label =
	  gtk_label_new (_
			 ("0=high priority, 19=system is responsive while encoding"));
	gtk_container_add (GTK_CONTAINER (vbox3), label);

	/* High Quality */
	use_high_qual_ckbx =
	  gtk_check_button_new_with_label (_("High quality mode"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (use_high_qual_ckbx),
				      config.encoder.use_high_qual);
	gtk_container_add (GTK_CONTAINER (vbox), use_high_qual_ckbx);

	/* CRC */
	use_crc_ckbx =
	  gtk_check_button_new_with_label (_("Include CRC error protection"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (use_crc_ckbx),
				      config.encoder.use_crc);
	gtk_container_add (GTK_CONTAINER (vbox), use_crc_ckbx);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 8);

	/* Extra options entry */
	label = gtk_label_new (_("Extra Options: "));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	extra_options_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (extra_options_entry),
				  MAX_OPTIONS_LENGTH - 20);
	gtk_entry_set_text (GTK_ENTRY (extra_options_entry),
			    config.encoder.extra_options);

	gtk_box_pack_start (GTK_BOX (hbox), extra_options_entry, TRUE, TRUE,
			    0);

	label = gtk_label_new (" Mp3 ");
	label2 = gtk_label_new (" Mp3 ");
	gtk_notebook_append_page_menu (GTK_NOTEBOOK (notebook),
				       main_frame, label, label2);
	gtk_widget_show_all (main_frame);
	return 0;
      }

    case CW_OK:
      {
	int i, temp, cur_plugin, br;
	GtkWidget *item;
	char bitrate_switch[12];
	char vbr_qual_switch[5];

	memset (config.encoder.encoder, 0, sizeof (config.encoder.encoder));

	item = gtk_menu_get_active (GTK_MENU (encoder_menu));
	cur_plugin = 0;
	for (i = 0; i < num_plugins; i++)
	  {
	    if (item == plugins[i].menu_item)
	      cur_plugin = i;
	  }
	strcpy (config.encoder.encoder, plugins[cur_plugin].encoder);
	strcpy (config.encoder.plugin, plugins[cur_plugin].plugin);
	config.encoder.type = plugins[cur_plugin].encoding_type;

	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (use_varbitrate_ckbx)))
	  config.encoder.use_varbitrate = 1;
	else
	  config.encoder.use_varbitrate = 0;

	config.encoder.vbr_qual =
	  CLAMP ((int)
		 gtk_adjustment_get_value (GTK_ADJUSTMENT (vbr_qual_adj)), 0,
		 9);

	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (use_high_qual_ckbx)))
	  config.encoder.use_high_qual = 1;
	else
	  config.encoder.use_high_qual = 0;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (use_crc_ckbx)))
	  config.encoder.use_crc = 1;
	else
	  config.encoder.use_crc = 0;

	for (temp = -1, i = 0; i < num_bitrates && temp < 0; i++)
	  {
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (bitrate[i].button)))
	      {
		temp = i;
	      }
	  }
	if (temp == -1)
	  temp = DEFAULT_BIT_RATE;

	config.encoder.bitrate = bitrate[temp].bitrate;

	strncpy (config.encoder.extra_options,
		 gtk_entry_get_text (GTK_ENTRY (extra_options_entry)),
		 MAX_OPTIONS_LENGTH);

	/* support special cases */
	br = config.encoder.bitrate;
	if (!strcmp (config.encoder.encoder, "l3enc"))
	  {
	    if (br == 160)
	      br = 128;
	    if (br == 320)
	      br = 256;

	    br *= 1000;
	    snprintf (bitrate_switch, sizeof (bitrate_switch), "%s %d",
		      plugins[cur_plugin].bitrate_op, br);
	  }
	else if (!strcmp (config.encoder.encoder, "mp3enc"))
	  {
	    br *= 1000;
	    snprintf (bitrate_switch, sizeof (bitrate_switch), "%s %d",
		      plugins[cur_plugin].bitrate_op, br);
	  }
	else if (!strcmp (config.encoder.encoder, "xingmp3enc"))
	  {
	    if (config.encoder.use_varbitrate)
	      {
		switch (br)
		  {
		  case 112:
		    br = 30;
		    break;
		  case 128:
		    br = 50;
		    break;
		  case 160:
		    br = 75;
		    break;
		  case 192:
		    br = 100;
		    break;
		  case 256:
		    br = 125;
		    break;
		  case 320:
		    br = 150;
		    break;
		  default:
		    br = 1;
		    break;
		  }
		snprintf (bitrate_switch, sizeof (bitrate_switch), "%s %d",
			  "-V", br);
	      }
	    else
	      snprintf (bitrate_switch, sizeof (bitrate_switch), "%s %d",
			"-B", br);
	  }
	else
	  snprintf (bitrate_switch, sizeof (bitrate_switch), "%s %d",
		    plugins[cur_plugin].bitrate_op, br);

	if (plugins[cur_plugin].vbr_qual_op
	    && strlen (plugins[cur_plugin].vbr_qual_op))
	  {
	    snprintf (vbr_qual_switch, 5, "%s %d",
		      plugins[cur_plugin].vbr_qual_op,
		      config.encoder.vbr_qual);
	  }
	else
	  {
	    sprintf (vbr_qual_switch, " ");
	  }

	if (!strcmp (config.encoder.encoder, "flac"))
	  {
	    snprintf (config.encoder.full_command, MAX_COMMAND_LENGTH, "%s",
		      config.encoder.encoder);
	  }
	else if (!strcmp (config.encoder.encoder, "mppenc"))
	  {
	    if (config.encoder.use_high_qual)
	      snprintf (config.encoder.full_command, MAX_COMMAND_LENGTH,
			"%s --overwrite %s %s",
			config.encoder.encoder,
			plugins[cur_plugin].high_qual_op,
			config.encoder.extra_options);
	    else
	      snprintf (config.encoder.full_command, MAX_COMMAND_LENGTH,
			"%s --overwrite %s %s",
			config.encoder.encoder,
			plugins[cur_plugin].vbr_qual_op,
			config.encoder.extra_options);
	  }
	else
	  {
	    snprintf (config.encoder.full_command, MAX_COMMAND_LENGTH,
		      "%s %s %s %s %s %s %s",
		      config.encoder.encoder,
		      bitrate_switch,
		      config.encoder.use_varbitrate ? plugins[cur_plugin].
		      varbitrate_op : "",
		      config.encoder.use_high_qual ? plugins[cur_plugin].
		      high_qual_op : "",
		      config.encoder.use_crc ? plugins[cur_plugin].
		      crc_op : "",
		      config.encoder.use_varbitrate ? vbr_qual_switch : "",
		      config.encoder.extra_options);
	  }
	return 0;
      }
    }
  /* Just to avoid warning */
  return 0;
}

int
cw_players_handler (int ops, _main_data * main_data, GtkWidget * notebook)
{
  static GtkWidget *main_frame;
  static GtkWidget *cd_play_entry, *cd_stop_entry;
  static GtkWidget *wav_entry, *mp3_entry;
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  switch (ops)
    {
    case WIDGET_CREATE:
      {
	GtkWidget *table, *label, *label2;

	main_frame = gtk_frame_new (_("Players"));
	gtk_container_set_border_width (GTK_CONTAINER (main_frame), 10);

	table = gtk_table_new (4, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);
	gtk_container_add (GTK_CONTAINER (main_frame), table);

	label = gtk_label_new (_("CD play command: "));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 0, 0, 0, 0);
	cd_play_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (cd_play_entry),
				  MAX_COMMAND_LENGTH - 2);
	gtk_entry_set_text (GTK_ENTRY (cd_play_entry),
			    config.cd_player.play_command);
	gtk_table_attach (GTK_TABLE (table), cd_play_entry, 1, 2, 0, 1,
			  GTK_EXPAND | GTK_FILL, 0, 0, 0);

	label = gtk_label_new (_("CD stop command: "));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 0, 0, 0, 0);
	cd_stop_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (cd_stop_entry),
				  MAX_COMMAND_LENGTH - 2);
	gtk_entry_set_text (GTK_ENTRY (cd_stop_entry),
			    config.cd_player.stop_command);
	gtk_table_attach (GTK_TABLE (table), cd_stop_entry, 1, 2, 1, 2,
			  GTK_EXPAND | GTK_FILL, 0, 0, 0);

	label = gtk_label_new (_("Wav play command: "));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, 0, 0, 0, 0);
	wav_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (wav_entry),
				  MAX_COMMAND_LENGTH - 2);
	gtk_entry_set_text (GTK_ENTRY (wav_entry), config.wav_player.command);
	gtk_table_attach (GTK_TABLE (table), wav_entry, 1, 2, 2, 3,
			  GTK_EXPAND | GTK_FILL, 0, 0, 0);

	label = gtk_label_new (_("Mp3 play command: "));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4, 0, 0, 0, 0);
	mp3_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (mp3_entry),
				  MAX_COMMAND_LENGTH - 2);
	gtk_entry_set_text (GTK_ENTRY (mp3_entry), config.mp3_player.command);
	gtk_table_attach (GTK_TABLE (table), mp3_entry, 1, 2, 3, 4,
			  GTK_EXPAND | GTK_FILL, 0, 0, 0);

	label = gtk_label_new (_("Players"));
	label2 = gtk_label_new (_("Players"));
	gtk_notebook_append_page_menu (GTK_NOTEBOOK (notebook),
				       main_frame, label, label2);
	gtk_widget_show_all (main_frame);
	return 0;
      }

    case CW_OK:
      {
	if (strlen (gtk_entry_get_text (GTK_ENTRY (cd_play_entry))) == 0
	    || strlen (gtk_entry_get_text (GTK_ENTRY (cd_stop_entry))) == 0
	    || strlen (gtk_entry_get_text (GTK_ENTRY (wav_entry))) == 0
	    || strlen (gtk_entry_get_text (GTK_ENTRY (mp3_entry))) == 0)
	  {
	    err_handler (GTK_WINDOW(main_window), EMPTY_ENTRY_ERR,
			 _("You need to fill every entry in players page"));
	    return -1;
	  }
	strcpy (config.cd_player.play_command,
		gtk_entry_get_text (GTK_ENTRY (cd_play_entry)));
	strcpy (config.cd_player.stop_command,
		gtk_entry_get_text (GTK_ENTRY (cd_stop_entry)));
	strcpy (config.wav_player.command,
		gtk_entry_get_text (GTK_ENTRY (wav_entry)));
	strcpy (config.mp3_player.command,
		gtk_entry_get_text (GTK_ENTRY (mp3_entry)));
	return 0;
      }
    }
  /* Just to avoid warning */
  return -1;
}

int
cw_files_handler (int ops, _main_data * main_data, GtkWidget * notebook)
{
  static GtkWidget *main_frame;
  static GtkWidget *format_string_entry, *dir_format_string_entry;
  static GtkWidget *convert_spaces_ckbx, *make_directories_ckbx,
    *create_id3_ckbx, *create_playlist_ckbx;
  static GtkWidget *table, *vbox, *hbox;

  switch (ops)
    {
    case WIDGET_CREATE:
      {
	GtkWidget *label, *label2;

	main_frame = gtk_frame_new (_("File Configuration"));
	gtk_container_set_border_width (GTK_CONTAINER (main_frame), 10);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

	/* convert spaces */
	convert_spaces_ckbx =
	  gtk_check_button_new_with_label (_
					   ("Convert spaces to underscores"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (convert_spaces_ckbx),
				      config.cddb_config.convert_spaces);
	gtk_box_pack_start (GTK_BOX (vbox), convert_spaces_ckbx, FALSE, FALSE,
			    0);

	/* make directories */
	make_directories_ckbx =
	  gtk_check_button_new_with_label (_
					   ("Create album subdirectory for each CD"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (make_directories_ckbx),
				      config.cddb_config.make_directories);
	gtk_box_pack_start (GTK_BOX (vbox), make_directories_ckbx, FALSE,
			    FALSE, 0);

	/* Create ID3 tag */
	create_id3_ckbx =
	  gtk_check_button_new_with_label (_("Create ID3 tag"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (create_id3_ckbx),
				      config.cddb_config.create_id3);
	gtk_box_pack_start (GTK_BOX (vbox), create_id3_ckbx, FALSE, FALSE, 0);

	/* Create m3u playlist file */
	create_playlist_ckbx =
	  gtk_check_button_new_with_label (_("Create m3u playlist"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (create_playlist_ckbx),
				      config.cddb_config.create_playlist);
	gtk_box_pack_start (GTK_BOX (vbox), create_playlist_ckbx, FALSE,
			    FALSE, 0);

	/* format string entry */
	hbox = gtk_hbox_new (FALSE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new (_("Filename format string: "));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	format_string_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (format_string_entry), 50);
	gtk_entry_set_text (GTK_ENTRY (format_string_entry),
			    config.cddb_config.format_string);
	gtk_box_pack_start (GTK_BOX (hbox), format_string_entry, FALSE, FALSE,
			    0);

	/* directory format string entry */
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 8);

	label = gtk_label_new (_("Directory format string: "));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	dir_format_string_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (dir_format_string_entry), 50);
	gtk_entry_set_text (GTK_ENTRY (dir_format_string_entry),
			    config.cddb_config.dir_format_string);
	gtk_box_pack_start (GTK_BOX (hbox), dir_format_string_entry, FALSE,
			    FALSE, 0);

	/* format string help */
	table = gtk_table_new (3, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);
	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	label = gtk_label_new (_("%a = Artist"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			  GTK_EXPAND | GTK_FILL, 0, 0, 0);
	label = gtk_label_new (_("%v = Album"));
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
			  GTK_EXPAND | GTK_FILL, 0, 0, 0);
	label = gtk_label_new (_("%# = Track no."));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
			  GTK_EXPAND | GTK_FILL, 0, 0, 0);
	label = gtk_label_new (_("%s = Song title"));
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, 1, 2,
			  GTK_EXPAND | GTK_FILL, 0, 0, 0);
	label = gtk_label_new (_("%y = Year"));
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			  GTK_EXPAND | GTK_FILL, 0, 0, 0);

	gtk_container_add (GTK_CONTAINER (main_frame), vbox);

	label = gtk_label_new (_("Files"));
	label2 = gtk_label_new (_("Files"));
	gtk_notebook_append_page_menu (GTK_NOTEBOOK (notebook),
				       main_frame, label, label2);
	gtk_widget_show_all (main_frame);
	return 0;
      }

    case CW_OK:
      if (gtk_toggle_button_get_active
	  (GTK_TOGGLE_BUTTON (convert_spaces_ckbx)))
	config.cddb_config.convert_spaces = 1;
      else
	config.cddb_config.convert_spaces = 0;

      if (gtk_toggle_button_get_active
	  (GTK_TOGGLE_BUTTON (make_directories_ckbx)))
	config.cddb_config.make_directories = 1;
      else
	config.cddb_config.make_directories = 0;

      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (create_id3_ckbx)))
	config.cddb_config.create_id3 = 1;
      else
	config.cddb_config.create_id3 = 0;

      if (gtk_toggle_button_get_active
	  (GTK_TOGGLE_BUTTON (create_playlist_ckbx)))
	config.cddb_config.create_playlist = 1;
      else
	config.cddb_config.create_playlist = 0;

      strcpy (config.cddb_config.format_string,
	      gtk_entry_get_text (GTK_ENTRY (format_string_entry)));

      strcpy (config.cddb_config.dir_format_string,
	      gtk_entry_get_text (GTK_ENTRY (dir_format_string_entry)));

      return 0;
    }
  /* Just to avoid warning */
  return -1;
}

int
cw_cddb_handler (int ops, _main_data * main_data, GtkWidget * notebook)
{
  static GtkWidget *main_frame;
  static GtkWidget *cddb_server_entry, *cddb_port_entry, *cddb_path_entry;
  static GtkWidget *use_http_ckbx, *cddb_proxy_server_entry,
    *cddb_proxy_port_entry;
  static GtkWidget *auto_lookup_ckbx;
  static GtkWidget *table, *vbox, *hbox;
  char *cddb_port_num, *cddb_proxy_port_num;

  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  switch (ops)
    {
    case WIDGET_CREATE:
      {
	GtkWidget *label, *label2;

	main_frame = gtk_frame_new (_("CDDB Configuration"));
	gtk_container_set_border_width (GTK_CONTAINER (main_frame), 10);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

	/* cddb server config table */
	table = gtk_table_new (6, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 3);
	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	/* URL box */
	label = gtk_label_new (_("URL: "));
	cddb_server_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (cddb_server_entry),
				  MAX_COMMAND_LENGTH - 2);
	gtk_entry_set_text (GTK_ENTRY (cddb_server_entry),
			    config.cddb_config.server);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (table), cddb_server_entry, 1, 2, 0, 1,
			  GTK_EXPAND | GTK_FILL, 0, 0, 0);

	/* port box */
	label = gtk_label_new (_("Port: "));
	cddb_port_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (cddb_port_entry), 5);
	cddb_port_num = int2str (config.cddb_config.port);
	gtk_entry_set_text (GTK_ENTRY (cddb_port_entry), cddb_port_num);
	free (cddb_port_num);
	cddb_port_num = NULL;
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (table), cddb_port_entry, 1, 2, 1, 2,
			  GTK_EXPAND | GTK_FILL, 0, 0, 0);

	/* use http */
	use_http_ckbx = gtk_check_button_new_with_label (_("Use HTTP"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (use_http_ckbx),
				      config.cddb_config.use_http);
	gtk_table_attach (GTK_TABLE (table), use_http_ckbx, 0, 2, 2, 3,
			  0, 0, 0, 0);

	/* Proxy URL box */
	label = gtk_label_new (_("Proxy Server: "));
	cddb_proxy_server_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (cddb_proxy_server_entry),
				  MAX_COMMAND_LENGTH - 2);
	gtk_entry_set_text (GTK_ENTRY (cddb_proxy_server_entry),
			    config.cddb_config.proxy_server);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (table), cddb_proxy_server_entry, 1, 2, 3,
			  4, GTK_EXPAND | GTK_FILL, 0, 0, 0);

	/* proxy port box */
	label = gtk_label_new (_("Proxy Port: "));
	cddb_proxy_port_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (cddb_proxy_port_entry), 5);
	cddb_proxy_port_num = int2str (config.cddb_config.proxy_port);
	gtk_entry_set_text (GTK_ENTRY (cddb_proxy_port_entry),
			    cddb_proxy_port_num);
	free (cddb_proxy_port_num);
	cddb_proxy_port_num = NULL;

	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (table), cddb_proxy_port_entry, 1, 2, 4,
			  5, GTK_EXPAND | GTK_FILL, 0, 0, 0);

	/* end cddb server config table */

	/* CDDB Path */
	hbox = gtk_hbox_new (FALSE, 3);
	label = gtk_label_new (_("CDDB Cache path: "));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	cddb_path_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (cddb_path_entry),
				  MAX_FILE_NAME_LENGTH - 3);
	gtk_entry_set_text (GTK_ENTRY (cddb_path_entry), config.cddb_path);
	gtk_widget_show (cddb_path_entry);

	gtk_box_pack_start (GTK_BOX (hbox), cddb_path_entry, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	/* CDDB Autolookup */
	hbox = gtk_hbox_new (FALSE, 3);
	auto_lookup_ckbx =
	  gtk_check_button_new_with_label (_("Automatic lookup on startup"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (auto_lookup_ckbx),
				      config.cddb_config.auto_lookup);
	gtk_widget_show (auto_lookup_ckbx);
	gtk_box_pack_start (GTK_BOX (hbox), auto_lookup_ckbx, FALSE, FALSE,
			    0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (main_frame), vbox);

	label = gtk_label_new ("CDDB");
	label2 = gtk_label_new ("CDDB");
	gtk_notebook_append_page_menu (GTK_NOTEBOOK (notebook),
				       main_frame, label, label2);
	gtk_widget_show_all (main_frame);
	return 0;
      }

    case CW_OK:
      if (strlen (gtk_entry_get_text (GTK_ENTRY (cddb_server_entry))) == 0
	  || strlen (gtk_entry_get_text (GTK_ENTRY (cddb_port_entry))) == 0)
	{
	  err_handler (GTK_WINDOW(main_window), EMPTY_ENTRY_ERR,
		       _("You need to specify a server and a port"));
	  return -1;
	}
      strcpy (config.cddb_config.server,
	      gtk_entry_get_text (GTK_ENTRY (cddb_server_entry)));

      config.cddb_config.port =
	atoi (gtk_entry_get_text (GTK_ENTRY (cddb_port_entry)));

      strcpy (config.cddb_config.proxy_server,
	      gtk_entry_get_text (GTK_ENTRY (cddb_proxy_server_entry)));

      config.cddb_config.proxy_port =
	atoi (gtk_entry_get_text (GTK_ENTRY (cddb_proxy_port_entry)));

      strcpy (config.cddb_path,
	      gtk_entry_get_text (GTK_ENTRY (cddb_path_entry)));

      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (use_http_ckbx)))
	config.cddb_config.use_http = 1;
      else
	config.cddb_config.use_http = 0;

      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (auto_lookup_ckbx)))
	config.cddb_config.auto_lookup = 1;
      else
	config.cddb_config.auto_lookup = 0;

      return 0;
    }
  /* Just to avoid warning */
  return -1;
}

void
cw_ok_button_clicked (GtkWidget * widget, gpointer callback_data)
{
  config_window_handler (CW_OK, NULL);
}

void
cw_cancel_button_clicked (GtkWidget * widget, gpointer callback_data)
{
  config_window_handler (CW_CANCEL, NULL);
}

void
config_window_handler (int ops, _main_data * main_data)
{
  static GtkWidget *window = NULL;
  static _main_data *saved_main_data;

  switch (ops)
    {
    case WIDGET_CREATE:
      {
	GtkWidget *vbox, *bbox, *notebook, *separator, *button;

	saved_main_data = main_data;
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	g_signal_connect (G_OBJECT (window), "destroy",
			  G_CALLBACK (cw_cancel_button_clicked), NULL);

	gtk_window_set_title (GTK_WINDOW (window), _("Configuration"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
	gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (notebook), 10);

	gtk_widget_realize (window);

	cw_general_handler (WIDGET_CREATE, main_data, notebook);
	cw_wav_handler (WIDGET_CREATE, main_data, notebook);
	cw_mp3_handler (WIDGET_CREATE, main_data, notebook);
	cw_players_handler (WIDGET_CREATE, main_data, notebook);
	cw_cddb_handler (WIDGET_CREATE, main_data, notebook);
	cw_files_handler (WIDGET_CREATE, main_data, notebook);

	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, TRUE, 10);

	bbox = gtk_hbox_new (TRUE, 5);
	gtk_container_set_border_width (GTK_CONTAINER (bbox), 10);
	gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, TRUE, 0);

	button = gtk_button_new_from_stock (GTK_STOCK_OK);
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (cw_ok_button_clicked), NULL);

	gtk_box_pack_end (GTK_BOX (bbox), button, TRUE, TRUE, 0);

	gtk_widget_set_can_default (button, TRUE);
	/*
	   FIXED: GTK_WIDGET_SET_FLAGS: deprecated since 2.22
	   GTK_WIDGET_SET_FLAGS( button, GTK_CAN_DEFAULT );
	 */

	button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (cw_cancel_button_clicked), NULL);

	gtk_box_pack_end (GTK_BOX (bbox), button, TRUE, TRUE, 0);

	gtk_widget_set_can_default (button, TRUE);
	/*
	   FIXED: GTK_WIDGET_SET_FLAGS: deprecated since 2.22
	   GTK_WIDGET_SET_FLAGS( button, GTK_CAN_DEFAULT );
	 */
	gtk_widget_grab_default (button);

	gtk_widget_show_all (window);
	return;
      }

    case CW_OK:
      main_data = saved_main_data;
      if (cw_general_handler (CW_OK, main_data, NULL) < 0
	  || cw_wav_handler (CW_OK, main_data, NULL) < 0
	  || cw_mp3_handler (CW_OK, main_data, NULL) < 0
	  || cw_players_handler (CW_OK, main_data, NULL) < 0
	  || cw_cddb_handler (CW_OK, main_data, NULL) < 0
	  || cw_files_handler (CW_OK, main_data, NULL) < 0)
	{
	  return;
	}

      gtk_widget_destroy (window);
      main_window_handler (MW_MODE_SELECT, 0, main_data);
#ifdef DEBUG
      printf ("==Debug Info==\n");
      printf ("wav path : <%s>,   mp3 path : <%s>\n",
	      config.wav_path, config.mp3_path);
      printf ("wav format : <%s>,   mp3 format : <%s>\n",
	      config.wav_file_name_format, config.mp3_file_name_format);
      printf ("prepend char : %c,   make mp3 from existing wav : %d\n",
	      config.prepend_char, config.make_mp3_from_existing_wav);
      printf ("--ripper     --\n");
      printf ("file : <%s>\nplugin : <%s>\n",
	      config.ripper.ripper, config.ripper.plugin);
      printf ("--encoder    --\n");
      printf ("file : <%s>\nplugin : <%s>\n",
	      config.encoder.encoder, config.encoder.plugin);
      printf ("full command : <%s>\n", config.encoder.full_command);
      printf ("--Cd player  --\n");
      printf ("Play : <%s>,   Stop : <%s>\n",
	      config.cd_player.play_command, config.cd_player.stop_command);
      printf ("--Wav Player --\n");
      printf ("Command : <%s>\n", config.wav_player.command);
      printf ("--Mp3 Player --\n");
      printf ("Command : <%s>\n", config.mp3_player.command);
#endif
      write_config ();
      return;

    case CW_CANCEL:
      gtk_widget_destroy (window);
      main_window_handler (MW_MODE_SELECT, 0, main_data);
      return;
    }
}
