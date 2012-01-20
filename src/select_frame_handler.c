/* Copyright (C) 2011
   Tony Mancill <tmancill@users.sourceforge.net>
   Dave Cinege <dcinege@psychosis.com>
   jos.dehaes@bigfoot.com
   Aljosha Papsch <papsch.al@googelmail.com>

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

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <string.h>

#include "id3.h"

#include "misc_utils.h"
#include "interface_common.h"
#include "main_window_handler.h"
#include "players_manipulation.h"

#include "select_frame_handler.h"

void sf_select_all_button_clicked (GtkWidget * widget,
				   gpointer callback_data);
void sf_select_button_toggled (GtkWidget * widget, gpointer callback_data);
void sf_filename_entry_changed (GtkWidget * widget, gpointer callback_data);
void sf_cd_play_button_clicked (GtkWidget * widget, gpointer callback_data);


void
sf_select_all_tbutton_clicked (GtkWidget * widget, gpointer callback_data)
{
  select_frame_handler (SF_SELECT_BUTTON_ACT_ALL, 0, NULL);
}

void
sf_track_selected_tbutton_toggled (GtkWidget * widget, gpointer callback_data)
{
  int track;

  track = *(int *) callback_data;
  select_frame_handler (SF_SELECT_BUTTON_ACT, track, NULL);
}

void
sf_artist_entry_changed (GtkWidget * widget, gpointer callback_data)
{
  select_frame_handler (SF_ARTIST_ENTRY_CHG, 0, NULL);
}

void
sf_album_entry_changed (GtkWidget * widget, gpointer callback_data)
{
  select_frame_handler (SF_ALBUM_ENTRY_CHG, 0, NULL);
}

// patch from M.Tyler
// tm:  we're not quite read for this (need updates to cddb.c)
void
sf_year_entry_changed (GtkWidget * widget, gpointer callback_data)
{
  select_frame_handler (SF_YEAR_ENTRY_CHG, 0, NULL);
}

void
sf_genre_entry_changed (GtkWidget * widget, gpointer callback_data)
{
  select_frame_handler (SF_GENRE_ENTRY_CHG, 0, NULL);
}

void
sf_filename_entry_changed (GtkWidget * widget, gpointer callback_data)
{
  int track;

  track = *(int *) callback_data;
  select_frame_handler (SF_FILENAME_ENTRY_CHG, track, NULL);
}

void
sf_cd_play_button_clicked (GtkWidget * widget, gpointer callback_data)
{
  char buf[5];

  play_cd_wav_mp3 (STOP, CD, NULL);
  play_cd_wav_mp3 (STOP, WAV, NULL);
  play_cd_wav_mp3 (STOP, MP3, NULL);

  snprintf (buf, sizeof (buf), "%d", (*(int *) callback_data) + 1);
  play_cd_wav_mp3 (PLAY, CD, buf);
}

void
select_frame_handler (int ops, int track, _main_data * main_data)
{
  static GtkWidget *select_frame;
  static GtkWidget *select_all_tbutton = NULL;
  static GtkWidget *rip_or_encode_combobox = NULL;
  static GtkWidget *artist_label = NULL;
  static GtkWidget *album_label = NULL;
  static GtkWidget *year_label = NULL;
  static GtkWidget *genre_label = NULL;
  static GtkWidget *artist_entry = NULL;
  static GtkWidget *album_entry = NULL;
  static GtkWidget *year_entry = NULL;
  static GtkWidget *genre_entry = NULL;
  /* Following are the widgets for the tracks. 
     GtkToggleButton: only selected tracks get ripped and/or encoded */
  static GtkWidget *track_selected_tbutton[MAX_NUM_TRACK];
  static GtkWidget *track_selected_tbutton_pixmap[MAX_NUM_TRACK];
  static GtkWidget *track_filename_entry[MAX_NUM_TRACK];
  static GtkWidget *track_play_pixmap[MAX_NUM_TRACK];
  static char saved_filename_entry[MAX_NUM_TRACK][MAX_FILE_NAME_LENGTH];
  static int track_numbers[MAX_NUM_TRACK];
  static _main_data *saved_main_data;
  static int num_tracks, button_state;

  switch (ops)
    {
    case WIDGET_CREATE:
      {
	GtkWidget *main_frame, *l1_vbox, *l2_hbox, *l2_scr_window, *l3_vbox,
	  *l4_hbox_artist, *l4_hbox_album, *l4_hbox_year_genre;
	GtkWidget *track_grid, *track_number_label, *track_length_label;
	GtkWidget *apply_image, *play_image;
	GtkWidget *play_button[MAX_NUM_TRACK];
	char track_number_buf[5];
	char *readable_length;

	if (select_frame != NULL)
	  {
	    return;
	  }

	main_frame = main_window_handler(MW_REQUEST_MF, 0, NULL);
	num_tracks = main_data->num_tracks;
	saved_main_data = main_data;

	/* Init section */
	l1_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	l2_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	l2_scr_window = gtk_scrolled_window_new(NULL, NULL);
	l3_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	l4_hbox_artist = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	l4_hbox_album = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	l4_hbox_year_genre = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	track_grid = gtk_grid_new();

	select_all_tbutton = gtk_toggle_button_new_with_label(_("Select All Tracks"));
	apply_image = gtk_image_new_from_stock(GTK_STOCK_APPLY,
					       GTK_ICON_SIZE_BUTTON);
	play_image = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
					      GTK_ICON_SIZE_BUTTON);
	rip_or_encode_combobox = gtk_combo_box_text_new();
	artist_label = gtk_label_new(_("Artist"));
	album_label = gtk_label_new(_("Album"));
	year_label = gtk_label_new(_("Year"));
	genre_label = gtk_label_new(_("Genre"));
	artist_entry = gtk_entry_new();
	album_entry = gtk_entry_new();
	year_entry = gtk_entry_new();
	genre_entry = gtk_entry_new();


	/* Config section */
	select_frame = l1_vbox;
	gtk_container_add(GTK_CONTAINER(main_frame), l1_vbox);
	gtk_box_set_homogeneous(GTK_BOX(l2_hbox), TRUE);
	gtk_button_set_image(GTK_BUTTON(select_all_tbutton), apply_image);
	gtk_button_set_image_position(GTK_BUTTON(select_all_tbutton), GTK_POS_LEFT);
	/* Probably better to use macros instead directly "rip" or "encode" */
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(rip_or_encode_combobox),
				  "rip", _("Rip to WAV"));
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(rip_or_encode_combobox),
				  "encode", _("Encode"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(rip_or_encode_combobox), 1);
	gtk_widget_set_size_request(artist_label, 100, -1);
	gtk_widget_set_size_request(album_label, 100, -1);
	gtk_widget_set_size_request(year_label, 100, -1);
	gtk_widget_set_hexpand(artist_entry, TRUE);
	gtk_widget_set_hexpand(album_entry, TRUE);
	gtk_widget_set_hexpand(genre_entry, TRUE);
	gtk_widget_set_hexpand(artist_entry, TRUE);

	gtk_box_pack_start(GTK_BOX(l4_hbox_year_genre), year_label,
			   FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(l4_hbox_year_genre), year_entry,
			   FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(l4_hbox_year_genre), genre_label,
			   FALSE, FALSE, 5);
	gtk_box_pack_end(GTK_BOX(l4_hbox_year_genre), genre_entry,
			 TRUE, TRUE, 5);
 	gtk_box_pack_start(GTK_BOX(l4_hbox_album), album_label,
			   FALSE, FALSE, 5);
	gtk_box_pack_end(GTK_BOX(l4_hbox_album), album_entry,
			 TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(l4_hbox_artist), artist_label,
			   FALSE, FALSE, 5);
	gtk_box_pack_end(GTK_BOX(l4_hbox_artist), artist_entry,
			 TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(l3_vbox), l4_hbox_artist,
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(l3_vbox), l4_hbox_album,
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(l3_vbox), l4_hbox_year_genre,
			   FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(l3_vbox), track_grid,
			 TRUE, TRUE, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(l2_scr_window),
					      l3_vbox);
	gtk_box_pack_start(GTK_BOX(l2_hbox), select_all_tbutton,
			   FALSE, FALSE, 5);
	gtk_box_pack_end(GTK_BOX(l2_hbox), rip_or_encode_combobox,
			 FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(l1_vbox), l2_hbox,
			   FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(l1_vbox), l2_scr_window,
			 TRUE, TRUE, 0);

	/* signal handler */
	g_signal_connect(G_OBJECT(select_all_tbutton), "clicked",
			 G_CALLBACK(sf_select_all_tbutton_clicked), NULL);
	g_signal_connect(G_OBJECT(artist_entry), "changed",
			 G_CALLBACK(sf_artist_entry_changed), NULL);
	g_signal_connect(G_OBJECT(album_entry), "changed",
			 G_CALLBACK(sf_album_entry_changed), NULL);
	g_signal_connect(G_OBJECT(year_entry), "changed",
			 G_CALLBACK(sf_year_entry_changed), NULL);
	g_signal_connect(G_OBJECT(genre_entry), "changed",
			 G_CALLBACK(sf_genre_entry_changed), NULL);

	/* Now the loop for the song entries. */

	for (int num_track = 0; num_track < num_tracks; num_track++)
	  {
	    track_numbers[num_track] = num_track;
	    if (strlen(saved_filename_entry[num_track]) == 0)
	      {
		get_track_title(saved_filename_entry[num_track], main_data, num_track);
	      }
	    
	    /* Init section */
	    track_selected_tbutton[num_track] = gtk_toggle_button_new();
	    play_button[num_track] = gtk_button_new();
	    track_selected_tbutton_pixmap[num_track] =
	      gtk_image_new_from_stock(GTK_STOCK_ADD,
				       GTK_ICON_SIZE_BUTTON);
	    track_play_pixmap[num_track] =
	      gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
				       GTK_ICON_SIZE_BUTTON);
	    track_filename_entry[num_track] = gtk_entry_new();
	    track_number_label = gtk_label_new("-");
	    track_length_label = gtk_label_new("--:--");

	    /* Config section
	       Play button and select track toggle button */
	    gtk_container_add(GTK_CONTAINER(track_selected_tbutton[num_track]),
			      track_selected_tbutton_pixmap[num_track]);
	    gtk_container_add(GTK_CONTAINER(play_button[num_track]),
			      track_play_pixmap[num_track]);
	    /* Track number */
	    sprintf(track_number_buf, "%d", num_track+1);
	    gtk_label_set_label(GTK_LABEL(track_number_label), track_number_buf);
	    /* Track length */
	    readable_length = length_to_readable(main_data->track[num_track].length);
	    gtk_label_set_label(GTK_LABEL(track_length_label), readable_length);
	    /* file name entry */
	    gtk_entry_set_max_length(GTK_ENTRY(track_filename_entry[num_track]),
				     MAX_FILE_NAME_LENGTH);
	    gtk_entry_set_text(GTK_ENTRY(track_filename_entry[num_track]),
			       saved_filename_entry[num_track]);
	    gtk_widget_set_hexpand(track_filename_entry[num_track], TRUE);

	    gtk_grid_attach(GTK_GRID(track_grid), play_button[num_track],
			    0, num_track, 1, 1);
	    gtk_grid_attach(GTK_GRID(track_grid), track_selected_tbutton[num_track],
			    1, num_track, 1, 1);
	    gtk_grid_attach(GTK_GRID(track_grid), track_number_label,
			    2, num_track, 1, 1);
	    gtk_grid_attach(GTK_GRID(track_grid), track_length_label,
			    3, num_track, 1, 1);
	    gtk_grid_attach(GTK_GRID(track_grid), track_filename_entry[num_track],
			    4, num_track, 1, 1);

	    /* Signal handler */
	    g_signal_connect(G_OBJECT(play_button[num_track]), "clicked",
			     G_CALLBACK(sf_cd_play_button_clicked),
			     &track_numbers[num_track]);
	    g_signal_connect(G_OBJECT(track_selected_tbutton[num_track]), "clicked",
			     G_CALLBACK(sf_track_selected_tbutton_toggled),
			     &track_numbers[num_track]);
	    g_signal_connect(G_OBJECT(track_filename_entry[num_track]), "changed",
			     G_CALLBACK(sf_filename_entry_changed),
			     &track_numbers[num_track]);
	  }

	gtk_widget_show_all(l1_vbox);

	return;
      }

    case WIDGET_DESTROY:
      if (select_frame == NULL)
	return;
      gtk_widget_destroy (select_frame);
      select_frame = NULL;
      return;

    case CLEAR_ENTRIES:
      /* clear filename entries */
      for (track = 0; track < num_tracks; track++)
	saved_filename_entry[track][0] = 0;
      return;

    case SF_SELECT_BUTTON_ACT:
      if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(track_selected_tbutton[track])))
	{
	  if (strlen (saved_filename_entry[track]) == 0)
	    {
	      get_track_title (saved_filename_entry[track], saved_main_data,
			       track);
	    }
	  gtk_entry_set_text (GTK_ENTRY (track_filename_entry[track]),
			      saved_filename_entry[track]);

	  gtk_widget_destroy (track_selected_tbutton_pixmap[track]);
	  track_selected_tbutton_pixmap[track] =
	    gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_BUTTON);
	  gtk_container_add (GTK_CONTAINER (track_selected_tbutton[track]),
			     track_selected_tbutton_pixmap[track]);
	  gtk_widget_show (track_selected_tbutton_pixmap[track]);
	}
      else
	{
	  gtk_widget_destroy (track_selected_tbutton_pixmap[track]);
	  track_selected_tbutton_pixmap[track] =
	    gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON);   
	  gtk_container_add (GTK_CONTAINER (track_selected_tbutton[track]),
			     track_selected_tbutton_pixmap[track]);
	  gtk_widget_show (track_selected_tbutton_pixmap[track]);
	}
      return;

    case SF_ARTIST_ENTRY_CHG:
      /* The artist name has been edited. Sync saved artist name */
      strcpy (saved_main_data->disc_artist,
	      gtk_entry_get_text (GTK_ENTRY (artist_entry)));
      return;

    case SF_ALBUM_ENTRY_CHG:
      /* The album name has been edited. Sync saved album name */
      strcpy (saved_main_data->disc_title,
	      gtk_entry_get_text (GTK_ENTRY (album_entry)));
      return;

    case SF_YEAR_ENTRY_CHG:
      /* The album year has been edited. Sync saved album year */
      strcpy (saved_main_data->disc_year,
	      gtk_entry_get_text (GTK_ENTRY (year_entry)));
      return;

    case SF_GENRE_ENTRY_CHG:
      /* The album genre has been edited. Sync saved album genre */
      strcpy (saved_main_data->disc_category,
	      gtk_entry_get_text (GTK_ENTRY (genre_entry)));
      return;

    case SF_FILENAME_ENTRY_CHG:
      if (strlen (gtk_entry_get_text (GTK_ENTRY (track_filename_entry[track]))) ==
	  0)
	{
	  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(track_selected_tbutton[track])))
	    {
	      /* The user has deleted the file name. Untoggle the button and
	       * delete saved_filename_entry too */
	      saved_filename_entry[track][0] = '\0';
	      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					    (track_selected_tbutton[track]),
					    FALSE);
	    }
	}
      else
	{
	  /* The file name has been edited. Sync saved file name */
	  strcpy (saved_filename_entry[track],
		  gtk_entry_get_text (GTK_ENTRY (track_filename_entry[track])));
	  /* 
	   * 20051030/tm
	   * no longer auto-select the track when the text is altered
	   * 

	   if ( GTK_TOGGLE_BUTTON( track_selected_button[ track ] ) ->active == FALSE )
	   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( track_selected_button[ track ] ),
	   TRUE );
	   */
	}
      return;

    case SF_SELECT_BUTTON_ACT_ALL:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (select_all_tbutton),
				    gtk_toggle_button_get_active
				    (GTK_TOGGLE_BUTTON(select_all_tbutton)));
      for (track = 0; track < num_tracks; track++)
	{
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					(track_selected_tbutton[track]),
					gtk_toggle_button_get_active
					(GTK_TOGGLE_BUTTON(select_all_tbutton)));
	}
      return;

      // auto-select all tracks     
    case SF_SELECT_ALL:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (select_all_tbutton),
				    TRUE);
      for (track = 0; track < num_tracks; track++)
	{
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					(track_selected_tbutton[track]), TRUE);
	}
      return;

    case SF_SYNC_MAIN_DATA:
      main_data = saved_main_data;

      for (track = 0; track < num_tracks; track++)
	{
	  char *tmp = saved_filename_entry[track];

	  if (strlen (tmp))
	    {

	      /* hack for l3enc. It is so stupid about spaces in the file name */
	      if (!strcmp (config.encoder.encoder, "l3enc"))
		convert_spaces (tmp, '_');

	      /* put track title back into main_data */
	      put_track_title (tmp, main_data, track);
	    }

	  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON
					   (track_selected_tbutton[track])))
	    {
	      const char * comp_str = gtk_combo_box_get_active_id
		(GTK_COMBO_BOX(rip_or_encode_combobox));

	      if (g_strcmp0(comp_str, "rip") == 0)
		{
		  main_data->track[track].make_wav = TRUE;
		  main_data->track[track].make_mp3 = FALSE;
		}
	      else
		{
		  main_data->track[track].make_wav = FALSE;
		  main_data->track[track].make_mp3 = TRUE;
		}

	    }
	}
      return;

    case SF_SYNC_SELECT_FRAME:

      main_data = saved_main_data;

      gtk_entry_set_text (GTK_ENTRY (artist_entry), main_data->disc_artist);
      gtk_entry_set_text (GTK_ENTRY (album_entry), main_data->disc_title);
      gtk_entry_set_text (GTK_ENTRY (year_entry), main_data->disc_year);
      gtk_entry_set_text (GTK_ENTRY (genre_entry), main_data->disc_category);

      for (track = 0; track < num_tracks; track++)
	{
	  button_state =
	    gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (track_selected_tbutton[track]));
	  get_track_title (saved_filename_entry[track], main_data, track);
	  gtk_entry_set_text (GTK_ENTRY (track_filename_entry[track]),
			      saved_filename_entry[track]);
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					(track_selected_tbutton[track]),
					button_state);
	}
      return;
    }
}
