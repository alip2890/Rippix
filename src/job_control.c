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

/* file job_control.c
 *
 * does calling of the ripper or encoder
 * handles files
 *
 * Ralf Engels  10/06/1999  changed lock file to handle all errors
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>


#include <stdio.h>		/* for bezo debug */

#include <glib.h>
#include <glib/gi18n.h>
#include <errno.h>		/* for errno */
#include <sys/stat.h>		/* defines read and write flags */
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <gtk/gtk.h>

#include <id3.h>

#include "misc_utils.h"
#include "interface_common.h"
#include "id3.h"
#include "ripper_encoder_manipulation.h"
#include "main_window_handler.h"
#include "select_frame_handler.h"
#include "status_frame_handler.h"

#include "job_control.h"

#define CLEAR_PIPE_BUF_SIZE         512
#define COUNT_BEFORE_GET_AVG        10

#define CALC_START_SESSION          0
#define CALC_STOP_SESSION           1
#define CALC_START                  2
#define CALC_UPDATE                 3
#define CALC_STOP                   4
#define CALC_PAUSE                  5
#define CALC_CONT                   6

#define JC_T_START                  0
#define JC_T_UPDATE                 1
#define JC_T_STOP                   2

/* Dialog structure */
typedef struct
{
  char *title;
  char *msg;
} _dialog_data;


/* Function Prototypes */
int lock_file (char *file_name, int is_temp);

void calc_stat (_main_data * main_data, _stat * stat,
		unsigned current, int cur_track, int cur_type, int ops);
/* calculates statiscal info to report to the user */
// current is length_processes now

void job_finisher (_main_data * main_data);

int find_next_job (_main_data * main_data,
		   int cur_track, int cur_type,
		   int *next_track, int *next_type);
/* This function finds what to do next. type is WAV or MP3.
 * It returns -1 when there's no job left, 0 when a job's found */

void job_controller_timeout_start ();
int job_controller_timeout_update (gpointer anything);
void job_controller_timeout_stop ();

void
calc_stat (_main_data * main_data, _stat * stat,
	   unsigned current, int cur_track, int cur_type, int ops)
{
  static unsigned int total_wav_length, total_mp3_length;
  static unsigned int total_wav_length_remain, saved_total_wav_length_remain;
  static unsigned int total_mp3_length_remain, saved_total_mp3_length_remain;
  static unsigned int length_first_track;
  static unsigned int first_track;
  static unsigned int first_track_time_remain;
  static unsigned int length_last_track;
  static unsigned int last_track;
  static unsigned int last_track_time_remain;
  static float wav_ratio;
  static float mp3_ratio;
  static time_t session_start_time;
  static time_t wav_start_time;
  static time_t mp3_start_time;
  static time_t pause_time;
  static float wav_temp_ratio;
  static float mp3_temp_ratio;
  static unsigned wav_prev_length_processed;
  static unsigned mp3_prev_length_processed;
  static int count;
  static char *wav_file_path, *enc_file_path;
  time_t cur_time;
  unsigned wav_length_processed;
  unsigned mp3_length_processed;
  int track, type;
  float total_time, temp;

  switch (ops)
    {
    case CALC_START_SESSION:
      /* Reset stat structure */
      memset (stat, 0, sizeof (stat));
      stat->wav_track = -1;
      stat->mp3_track = -1;
      stat->tracks_remain = 0;
      stat->wav_progress = 0;
      stat->mp3_progress = 0;
      stat->total_progress = 0;
      stat->ripping = FALSE;
      stat->encoding = FALSE;

      wav_ratio = config.wav_ratio;
      mp3_ratio = config.mp3_ratio;
      total_wav_length_remain = 0;
      total_mp3_length_remain = 0;
      length_first_track = 0;
      length_last_track = 0;
      first_track = -1;
      last_track = -1;
      first_track_time_remain = 0;
      last_track_time_remain = 0;

      /* calculate total times */
      track = -1;
      type = WAV;
      while (find_next_job (main_data, track, WAV, &track, &type) >= 0)
	{
	  total_wav_length_remain += main_data->track[track].length;
	  stat->tracks_remain++;
	  if (length_first_track == 0)
	    {
	      first_track = track;
	      length_first_track = main_data->track[track].length;
	    }
	  last_track = track;
	  length_last_track = main_data->track[track].length;
	}
      first_track_time_remain =
	main_data->track[first_track].length * wav_ratio;
      last_track_time_remain =
	main_data->track[last_track].length * mp3_ratio;

      track = -1;
      type = MP3;
      while (find_next_job (main_data, track, MP3, &track, &type) >= 0)
	total_mp3_length_remain += main_data->track[track].length;

      total_wav_length = total_wav_length_remain;
      total_mp3_length = total_mp3_length_remain;
      saved_total_wav_length_remain = total_wav_length_remain;
      saved_total_mp3_length_remain = total_mp3_length_remain;
      session_start_time = time (NULL);
      return;

    case CALC_START:
      create_file_names_for_track (main_data, cur_track, &wav_file_path,
				   &enc_file_path);
      if (cur_type == WAV)
	{
	  strcpy (stat->dest_file_name,
		  file_name_without_path (wav_file_path));
	  wav_prev_length_processed = 0;
	  stat->wav_time_elapsed = 0;
	  stat->wav_track = cur_track;
	  wav_start_time = time (NULL);
	}
      else
	{
	  strcpy (stat->src_file_name,
		  file_name_without_path (wav_file_path));
	  strcpy (stat->dest_file_name,
		  file_name_without_path (enc_file_path));
	  mp3_prev_length_processed = 0;
	  stat->mp3_time_elapsed = 0;
	  stat->mp3_track = cur_track;
	  mp3_start_time = time (NULL);
	}
      count = 0;
      return;

    case CALC_UPDATE:
      cur_time = time (NULL);
      stat->total_time_elapsed = cur_time - session_start_time;

      /* avoid dividing by zero */
      if (current == 0)
	current = 1;

      if (cur_type == WAV)
	{
	  stat->wav_time_elapsed = cur_time - wav_start_time;
	  wav_length_processed = current;

	  stat->wav_progress =
	    (float) wav_length_processed / main_data->track[cur_track].length;
	  wav_temp_ratio =
	    (float) (stat->wav_time_elapsed) / wav_length_processed;

	  if (count++ >= COUNT_BEFORE_GET_AVG)
	    {
	      /* do not adjust ratio if length_processed is not 1 */
	      if (wav_length_processed != 1)
		wav_ratio = (wav_temp_ratio + 2 * wav_ratio) / 3;
	    }
	  else
	    wav_temp_ratio = wav_ratio;

	  stat->wav_time_remain =
	    (main_data->track[cur_track].length -
	     wav_length_processed) * wav_temp_ratio;
	  total_wav_length_remain -=
	    wav_length_processed - wav_prev_length_processed;
	  wav_prev_length_processed = wav_length_processed;

	  if (cur_track == first_track)
	    first_track_time_remain = stat->wav_time_remain;
	}
      else
	{
	  stat->mp3_time_elapsed = cur_time - mp3_start_time;
	  mp3_length_processed = current;

	  stat->mp3_progress =
	    (float) mp3_length_processed / main_data->track[cur_track].length;

	  mp3_temp_ratio =
	    (float) (stat->mp3_time_elapsed) / mp3_length_processed;

	  if (count++ >= COUNT_BEFORE_GET_AVG)
	    {
	      /* do not adjust ratio if length_processed is not 1 */
	      if (mp3_length_processed != 1)
		mp3_ratio = (mp3_temp_ratio + 2 * mp3_ratio) / 3;
	    }
	  else
	    {
	      mp3_temp_ratio = mp3_ratio;
	    }

	  stat->mp3_time_remain =
	    (main_data->track[cur_track].length -
	     mp3_length_processed) * mp3_temp_ratio;
	  total_mp3_length_remain -=
	    mp3_length_processed - mp3_prev_length_processed;
	  mp3_prev_length_processed = mp3_length_processed;

	  if (cur_track == last_track)
	    last_track_time_remain = stat->mp3_time_remain;
	}

      /* Total progress */
      if (main_data->track[cur_track].make_mp3)
	{
	  /* for ripping and encoding. Assumption: Total time is equal to
	     time_to_rip_first_track + time_to_encode_all_tracks */

	  if (mp3_ratio > wav_ratio)
	    {
	      stat->total_time_remain =
		first_track_time_remain + total_mp3_length_remain * mp3_ratio;
	      total_time =
		length_first_track * wav_ratio + total_mp3_length * mp3_ratio;
	    }
	  else
	    {
	      /* for rare case when ripping takes longer than encoding */
	      stat->total_time_remain =
		last_track_time_remain + total_wav_length_remain * wav_ratio;
	      total_time =
		length_last_track * mp3_ratio + total_wav_length * wav_ratio;
	    }
	}
      else
	{
	  /* if we are only ripping, only use the wav times */
	  stat->total_time_remain = total_wav_length_remain * wav_ratio;
	  total_time = total_wav_length * wav_ratio;
	}

      temp = (total_time - stat->total_time_remain) / total_time;
      if (temp >= stat->total_progress)
	stat->total_progress = temp;
      else if ((temp - stat->total_progress) / temp >= 0.05)
	stat->total_progress = temp;

      return;

    case CALC_PAUSE:
      pause_time = time (NULL);
      return;

    case CALC_CONT:
      pause_time = time (NULL) - pause_time;
      session_start_time += pause_time;
      wav_start_time += pause_time;
      mp3_start_time += pause_time;
      return;

    case CALC_STOP:
      if (cur_type == WAV)
	{
	  saved_total_wav_length_remain -=
	    main_data->track[stat->wav_track].length;
	  total_wav_length_remain = saved_total_wav_length_remain;
	}
      else
	{
	  saved_total_mp3_length_remain -=
	    main_data->track[stat->mp3_track].length;
	  total_mp3_length_remain = saved_total_mp3_length_remain;
	}
      return;

    case CALC_STOP_SESSION:
      stat->tracks_remain = 0;
      stat->tracks_done = 0;
      config.wav_ratio = wav_ratio;
      config.mp3_ratio = mp3_ratio;
      return;
    }
}

int
lock_file (char *file_name, int is_temp)
{
  int fd;
  char *temp;
  char buf[MAX_FILE_NAME_LENGTH];

  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  fd = open (file_name,
	     O_CREAT | O_EXCL,
	     S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);

  if (fd >= 0)
    {
      close (fd);
      unlink (file_name);
    }
  else
    {
      if (errno == EEXIST)
	if (config.ask_when_file_exists == FALSE || is_temp == TRUE)
	  {
	    if (config.make_mp3_from_existing_wav)
	      return 1;

	    /* Prepend config.prepend_char until we succeed open() */
	    if (strlen (file_name) >
		MAX_FILE_NAME_LENGTH + MAX_FILE_PATH_LENGTH - 2)
	      return -1;
	    else
	      {
		temp = file_name_without_path (file_name);
		strcpy (buf, temp);
		temp[0] = config.prepend_char;
		strcpy (temp + 1, buf);

		/* now try again */
		if (lock_file (file_name, is_temp) < 0)
		  return -1;
	      }
	  }
	else
	  {
	    if (config.make_mp3_from_existing_wav)
	      return 1;

	    if (dialog_handler (WIDGET_CREATE, FALSE,
				DL_OVERWRITE_CONFIRM,
				FALSE,
				file_name_without_path (file_name),
				NULL, 0) == TRUE)
	      /* Just return. Cdparanoia or 8hz-mp3 will overwrite on it */
	      return 0;

	    /* Let's ask the user what s/he wants */
	    temp = file_name_without_path (file_name);
	    strcpy (buf, temp);
	    if (dialog_handler (WIDGET_CREATE, TRUE,
				DL_ENTER_FILE_NAME,
				TRUE, buf, buf, sizeof (buf) - 1) == FALSE)
	      /* The user does not want to continue. return error */
	      return -1;
	    strcpy (temp, buf);

	    /* now try again */
	    if (lock_file (file_name, is_temp) < 0)
	      return -1;
	  }
      else			/* an other error (maybe directory not existent */
	{
	  err_handler (GTK_WINDOW(main_window), CREATING_FILE_ERROR, file_name);
	  return -1;
	}
    }
  return 0;
}

/* this is called when the go button is clicked. The default file names are determined
   and the files are locked. */
void
job_starter (_main_data * main_data)
{
  int i, track, type, is_temp, code;
  static char *wav_file_path, *enc_file_path;

  /* Sync main_data structure with the data user has entered */
  select_frame_handler (SF_SYNC_MAIN_DATA, 0, main_data);
  /* Setup directory and filename extension */
  if (!create_filenames_from_format (main_data))
    return;
  /* Reset exist flags */
  for (i = 0; i < main_data->num_tracks; i++)
    {
      if (main_data->track[i].make_wav)
	main_data->track[i].wav_exist = FALSE;
      if (main_data->track[i].make_mp3)
	main_data->track[i].mp3_exist = FALSE;
    }
  /* Lock target files */
  track = -1;
  type = WAV;
  while (find_next_job (main_data, track, WAV, &track, &type) >= 0)
    {
      is_temp = FALSE;
      if (!create_file_names_for_track
	  (main_data, track, &wav_file_path, NULL))
	return;
      code = lock_file (wav_file_path, is_temp);
      if (code < 0)
	return;
      if (code == 1)
	{
	  main_data->track[track].make_wav = FALSE;
	  main_data->track[track].wav_exist = TRUE;
	}
    }

  if (main_data->track[track].make_mp3)
    {
      track = -1;
      type = MP3;
      while (find_next_job (main_data, track, MP3, &track, &type) >= 0)
	{
	  if (!create_file_names_for_track
	      (main_data, track, NULL, &enc_file_path))
	    return;
	  if (lock_file (enc_file_path, FALSE) < 0)
	    return;
	}
    }

  /* Destroy select frame & change main window button state */
  select_frame_handler (WIDGET_DESTROY, 0, NULL);
  main_window_handler (MW_MODE_STATUS, NULL, NULL);
  /* Start Job */
  job_controller (JC_START, main_data);
}

/* called after all jobs are finshed. Files are cleaned up and renamed */
void
job_finisher (_main_data * main_data)
{
  int i;
  static char status_message[(MAX_FILE_PATH_LENGTH + MAX_FILE_NAME_LENGTH) *
			     MAX_NUM_TRACK];
  static char buffer[(MAX_FILE_PATH_LENGTH + MAX_FILE_NAME_LENGTH) *
		     MAX_NUM_TRACK];
  static char *wav_file, *enc_file;
  int madewavs = FALSE;
  int mademp3s = FALSE;
  int tracksdone = 0;
  char *s_track_num;
  char *artist;
  ID3Tag *myTag;

  FILE *fp_playlist = NULL;
  char playlist_filespec[MAX_FILE_PATH_LENGTH + MAX_FILE_NAME_LENGTH];

  /* Allocate space dynamically.  This is overkill, but certainly won't be a problem. */
  s_track_num = (char *) malloc ((main_data->num_tracks + 2) * sizeof (char));
  buffer[0] = 0;

  /* Clean up */
  for (i = 0; i < main_data->num_tracks; i++)
    {
      create_file_names_for_track (main_data, i, &wav_file, &enc_file);
      if (main_data->track[i].wav_exist == FALSE)
	unlink (wav_file);
      else if (main_data->track[i].make_wav == TRUE)
	{
	  madewavs = TRUE;
	  sprintf (&buffer[strlen (buffer)], "%d: %s\n", ++tracksdone,
		   file_name_without_path (wav_file));
	}
      main_data->track[i].make_wav = FALSE;

      if (main_data->track[i].mp3_exist == TRUE
	  && main_data->track[i].make_mp3 == TRUE)
	{
	  mademp3s = TRUE;

	  /* add ID3 tag if requested */
	  if (config.cddb_config.create_id3 == TRUE)
	    {
	      if (!(artist = main_data->track[i].artist))
		artist = main_data->disc_artist;

	      // TODO:  fix this to use something like
	      //              if (main_data->encoding_type == OGG) {

	      if (!strcmp (config.encoder.encoder, "oggenc"))
		{
		  /* set VORBIS tags using vorbistag - added DATE tag - R. Turnbull 1-2-2010 */
		  vorbistag (enc_file,
			     artist,
			     main_data->disc_title,
			     main_data->disc_year,
			     main_data->track[i].title,
			     id3_find_cddb_category (main_data->
						     disc_category), i + 1);
		}
	      else if (!strcmp (config.encoder.encoder, "flac"))
		{
		  /* set FLAC tags using metaflac R. Turnbull 1-2-2010 */
		  flactag (enc_file,
			   artist,
			   main_data->disc_title,
			   main_data->disc_year,
			   main_data->track[i].title,
			   id3_find_cddb_category (main_data->disc_category),
			   i + 1);
		}
	      else if (!strcmp (config.encoder.encoder, "mppenc"))
		{
		  /* do nothing for for musepack right now -
		     originally supported id3 now wants apev2 tags */
		}
	      else
		{
		  /* assume MP3 tag is desired */
		  sprintf (s_track_num, "%d", (i + 1));

		  myTag = ID3Tag_New ();
		  ID3Tag_Link (myTag, enc_file);

		  set_TagField (myTag, main_data->track[i].title,
				ID3FID_TITLE);
		  set_TagField (myTag, artist, ID3FID_LEADARTIST);
		  set_TagField (myTag, main_data->disc_title, ID3FID_ALBUM);
		  set_TagField (myTag, main_data->disc_year, ID3FID_YEAR);
		  set_TagField (myTag, s_track_num, ID3FID_TRACKNUM);
		  set_TagField (myTag, main_data->disc_category,
				ID3FID_CONTENTTYPE);
		  ID3Tag_UpdateByTagType (myTag, ID3TT_ID3V2);
		  ID3Tag_Delete (myTag);
		}
	    }

	  //dc: strcat() is for sissies!
	  sprintf (&buffer[strlen (buffer)], "%d: %s\n", ++tracksdone,
		   file_name_without_path (enc_file));

	  // tm: basic playlist support - thanks to Mark Tyler
	  if (config.cddb_config.create_playlist == TRUE)
	    {
	      if (fp_playlist == NULL)
		{
		  sprintf (playlist_filespec, "%s/playlist.m3u",
			   file_path_without_name (enc_file));
		  fp_playlist = fopen (playlist_filespec, "w");
		}

	      // if we succeeded above, we can now write to this
	      if (fp_playlist != NULL)
		fprintf (fp_playlist, "%s\n",
			 file_name_without_path (enc_file));
	    }
	}
      main_data->track[i].make_mp3 = FALSE;
    }				/* end loop over all tracks */
  free (s_track_num);

  if ((config.cddb_config.create_playlist == TRUE) && (fp_playlist != NULL))
    fclose (fp_playlist);

  /* Generate status message */
  sprintf (status_message,
	   _("Tracks Completed: %2d\n\nArtist: %s\nAlbum: %s\n\n%s"),
	   tracksdone, main_data->disc_artist, main_data->disc_title, buffer);

  /* show status pop up */
  if (madewavs)
    status_handler (STAT_FINISH_WAV, status_message);
  else if (mademp3s)
    status_handler (STAT_FINISH_MP3, status_message);

  /* Clear status bar */
  main_window_handler (MW_CLEAR_STATUSBAR, NULL, NULL);

  /* Destroy status widget */
  wm_status_frame_handler (WIDGET_DESTROY, WAV, NULL, NULL);

  /* Create select frame */
  select_frame_handler (WIDGET_CREATE, 0, main_data);
  main_window_handler (MW_MODE_SELECT, NULL, NULL);
}

/* finds the next track from cur_track. If cur_type is wav, it will find the next wav
   file to rip. If cur_type is mp3, it will find the next mp3 file to encode */
int
find_next_job (_main_data * main_data,
	       int cur_track, int cur_type, int *next_track, int *next_type)
{
  int flag, track;
  int temp_next, temp_type;

  /* Find next track from cur_track */
  flag = 1;
  track = cur_track + 1;

  while (track < main_data->num_tracks && flag)
    {
      if (main_data->track[track].make_wav
	  || main_data->track[track].make_mp3)
	flag = 0;
      else
	track++;
    }

  if (flag)
    return -1;

  /* for finding the next wav file, check to see if the file exists */
  if (cur_type == WAV && main_data->track[track].make_mp3
      && main_data->track[track].wav_exist == TRUE
      && config.make_mp3_from_existing_wav == TRUE)
    {
      if (find_next_job (main_data, track, WAV, &temp_next, &temp_type) >= 0)
	{
	  *next_track = temp_next;
	  *next_type = WAV;
	}
      else
	return -1;
    }
  else
    {
      *next_track = track;
      *next_type = MP3;

    }

  return 0;
}


static int timer;

void
job_controller_timeout_start ()
{
  timer = g_timeout_add (JC_TIMEOUT, job_controller_timeout_update, NULL);
}


int
job_controller_timeout_update (gpointer anything)
{
  job_controller (JC_UPDATE, NULL);
  return TRUE;
}


void
job_controller_timeout_stop ()
{
  g_source_remove (timer);
}


void
job_controller (int ops, _main_data * main_data)
{
  static int cur_type = -1;
  static int wav_cur_track = -1;
  static int mp3_cur_track = -1;
  static int mp3_nxt_track = -1;
  static pid_t wav_pg_pid = -1;
  static pid_t mp3_pg_pid = -1;
  static pid_t wav_pi_pid = -1;
  static pid_t mp3_pi_pid = -1;
  static int wav_read_fd;
  static int mp3_read_fd;
  static _stat stat;
  static _main_data *saved_main_data;
  static char *wav_file_path, *enc_file_path;
  unsigned wav_current = 0;
  unsigned mp3_current = 0;
  double wav_progress;
  double mp3_progress;
  char msg[MAX_PLUGIN_OUTPUT_LENGTH];
  char *str;
  int temp, temp_track;
  GtkWidget *main_window = main_window_handler (MW_REQUEST_MW, NULL, NULL);

  switch (ops)
    {
    case JC_START:
      /* called once when the go button is clicked. */
      saved_main_data = main_data;

      if (wav_cur_track != -1)
	{
	  err_handler (GTK_WINDOW(main_window), JOB_IN_PROGRESS_ERR, NULL);
	  job_finisher (main_data);
	  return;
	}

      if (find_next_job (main_data, wav_cur_track, WAV,
			 &wav_cur_track, &cur_type) < 0)
	{
	  /* ok, no more wavs to rip, try looking for mp3s */
	  if (find_next_job (main_data, mp3_cur_track, MP3,
			     &mp3_cur_track, &cur_type) < 0)
	    {
	      /* nothing at all found */
	      err_handler (GTK_WINDOW(main_window), NOTHING_TO_DO_ERR, NULL);
	      job_finisher (main_data);
	      return;
	    }
	  create_file_names_for_track (main_data, mp3_cur_track,
				       &wav_file_path, &enc_file_path);

	  calc_stat (main_data, &stat, mp3_current,
		     mp3_cur_track, MP3, CALC_START_SESSION);
	  calc_stat (main_data, &stat, mp3_current,
		     mp3_cur_track, MP3, CALC_START);

	  /* start encoding */
	  if (start_ripping_encoding
	      (MP3, main_data->track[mp3_cur_track].begin,
	       main_data->track[mp3_cur_track].length, mp3_cur_track,
	       wav_file_path, enc_file_path, &mp3_pg_pid, &mp3_pi_pid,
	       &mp3_read_fd) < 0)
	    {
	      job_finisher (main_data);
	      return;
	    }
	  stat.encoding = TRUE;
	}
      else
	{
	  /* found the first wav to rip */
	  create_file_names_for_track (main_data, wav_cur_track,
				       &wav_file_path, &enc_file_path);
	  calc_stat (main_data, &stat, wav_current, wav_cur_track, WAV,
		     CALC_START_SESSION);
	  calc_stat (main_data, &stat, wav_current, wav_cur_track, WAV,
		     CALC_START);

	  /* start ripping */
	  if (start_ripping_encoding
	      (WAV, main_data->track[wav_cur_track].begin,
	       main_data->track[wav_cur_track].length, wav_cur_track,
	       wav_file_path, enc_file_path, &wav_pg_pid, &wav_pi_pid,
	       &wav_read_fd) < 0)
	    {
	      job_finisher (main_data);
	      return;
	    }
	  stat.ripping = TRUE;

	  /* Find the next track to encode if any */
	  if (find_next_job
	      (main_data, mp3_cur_track, MP3, &mp3_nxt_track, &cur_type) < 0
	      || !main_data->track[mp3_nxt_track].make_mp3)
	    {
	      mp3_nxt_track = -1;
	    }
	}

      /* Create wav/mp3 status frame */
      wm_status_frame_handler (WIDGET_CREATE, cur_type, &stat, NULL);

      job_controller_timeout_start ();
      return;

    case JC_UPDATE:
      main_data = saved_main_data;

      /* Part 1: check progress on the rip */
      if (stat.ripping == TRUE)
	{
	  temp =
	    read_and_process_plugin_output (wav_read_fd, &wav_progress, msg);
	  wav_current = main_data->track[wav_cur_track].length * wav_progress;

	  switch (temp)
	    {
	    case PLUGIN_MSG_PARSE_ERR:
	      /* Nothing to do. Let's wait more */
	      break;

	    case PLUGIN_PROGRESS_MSG:
	      /* progress report, update status display */
	      calc_stat (main_data, &stat, wav_current,
			 wav_cur_track, WAV, CALC_UPDATE);
	      /* Update status widget */
	      if (msg[0] == '\0')
		str = NULL;
	      else
		str = msg;
	      wm_status_frame_handler (WIDGET_UPDATE, WAV, &stat, str);
	      break;

	    case PLUGIN_NO_MSG_AVAILABLE:
	      /* Check if the plugin has exited
	         It only happens when ripperX failed to execute the plugin */
	      if (wav_pi_pid >= 0)
		if (waitpid (wav_pi_pid, NULL, WNOHANG) == wav_pi_pid)
		  {
		    err_handler (GTK_WINDOW(main_window), PLUGIN_NOT_PRESENT_ERR,
				 _
				 ("Maybe ripperX has failed to execute the plugin"));
		    wav_pi_pid = -1;
		    job_controller_timeout_stop ();
		    job_controller (JC_ABORT_ALL_DELETE, main_data);
		    return;
		  }

	      /* Check if the job is finished */
	      if (waitpid (wav_pg_pid, NULL, WNOHANG) == wav_pg_pid)
		{
		  /* One job finished, go for next one */

		  /* kill the plugin */
		  if (waitpid (wav_pi_pid, NULL, WNOHANG) != wav_pi_pid)
		    {
		      kill (-wav_pi_pid, SIGTERM);
		      waitpid (wav_pi_pid, NULL, 0);
		    }

		  /* Close the fd */
		  close (wav_read_fd);

		  /* stop calculating stats */
		  calc_stat (main_data, &stat, wav_current,
			     wav_cur_track, WAV, CALC_STOP);

		  /* Mark that it exists */
		  main_data->track[wav_cur_track].wav_exist = TRUE;

		  /* find next track to rip */
		  temp_track = wav_cur_track;
		  if (find_next_job
		      (main_data, wav_cur_track, WAV, &wav_cur_track,
		       &cur_type) < 0)
		    {
		      /* All finished - no more rips */
		      wav_pg_pid = -1;
		      wav_pi_pid = -1;
		      wav_cur_track = -1;
		      cur_type = -1;
		      stat.ripping = FALSE;

		      /* if we are only ripping, finish up for good */
		      if (!main_data->track[temp_track].make_mp3)
			{
			  calc_stat (main_data, &stat, wav_current,
				     temp_track, WAV, CALC_STOP_SESSION);

			  job_controller_timeout_stop ();
			  job_finisher (main_data);
			  return;
			}
		    }
		  else
		    {
		      /* if we are only ripping, update the stats */
		      if (!main_data->track[temp_track].make_mp3)
			{
			  stat.tracks_done++;
			  stat.tracks_remain--;
			}

		      /*  start ripping the next track */
		      create_file_names_for_track (main_data, wav_cur_track,
						   &wav_file_path,
						   &enc_file_path);
		      calc_stat (main_data, &stat, wav_current, wav_cur_track,
				 WAV, CALC_START);
		      if (start_ripping_encoding
			  (WAV, main_data->track[wav_cur_track].begin,
			   main_data->track[wav_cur_track].length,
			   wav_cur_track, wav_file_path, enc_file_path,
			   &wav_pg_pid, &wav_pi_pid, &wav_read_fd) < 0)
			{
			  calc_stat (main_data, &stat, wav_current,
				     wav_cur_track, WAV, CALC_STOP_SESSION);
			  job_controller_timeout_stop ();
			  job_finisher (main_data);
			  return;
			}
		    }
		}		/* end if job is finished section */
	      break;
	    }			/* end rip switch */
	}			/* end rip progress check */

      /* Part 2: check progress on the encode */
      if (stat.encoding == TRUE)
	{
	  temp =
	    read_and_process_plugin_output (mp3_read_fd, &mp3_progress, msg);
	  mp3_current = main_data->track[mp3_cur_track].length * mp3_progress;

	  switch (temp)
	    {
	    case PLUGIN_MSG_PARSE_ERR:
	      /* Nothing to do. Let's wait more */
	      break;

	    case PLUGIN_PROGRESS_MSG:
	      /* progress report, update status display */
	      calc_stat (main_data, &stat, mp3_current,
			 mp3_cur_track, MP3, CALC_UPDATE);
	      /* Update status widget */
	      if (msg[0] == '\0')
		str = NULL;
	      else
		str = msg;
	      wm_status_frame_handler (WIDGET_UPDATE, MP3, &stat, str);
	      break;

	    case PLUGIN_NO_MSG_AVAILABLE:
	      /* Check if the plugin has exited
	         It only happens when ripperX failed to execute the plugin */
	      if (mp3_pi_pid >= 0)
		if (waitpid (mp3_pi_pid, NULL, WNOHANG) == mp3_pi_pid)
		  {
		    err_handler (GTK_WINDOW(main_window), PLUGIN_NOT_PRESENT_ERR,
				 _
				 ("Maybe ripperX has failed to execute the plugin"));
		    mp3_pi_pid = -1;
		    job_controller_timeout_stop ();
		    job_controller (JC_ABORT_ALL_DELETE, main_data);
		    return;
		  }

	      /* Check if the job is finished */
	      if (waitpid (mp3_pg_pid, NULL, WNOHANG) == mp3_pg_pid)
		{
		  /* One job finished, go for next one */

		  /* kill the plugin */
		  if (waitpid (mp3_pi_pid, NULL, WNOHANG) != mp3_pi_pid)
		    {
		      kill (-mp3_pi_pid, SIGTERM);
		      waitpid (mp3_pi_pid, NULL, 0);
		    }

		  /* Close the fd */
		  close (mp3_read_fd);

		  /* stop calculating stats */
		  calc_stat (main_data, &stat, mp3_current,
			     mp3_cur_track, MP3, CALC_STOP);

		  main_data->track[mp3_cur_track].mp3_exist = TRUE;
		  /* Delete WAV file if he/she doesn't want it */
		  if (!config.keep_wav)
		    {
		      create_file_names_for_track (main_data, mp3_cur_track,
						   &wav_file_path,
						   &enc_file_path);
		      if (unlink (wav_file_path) < 0)
			err_handler (GTK_WINDOW(main_window), FILE_DELETE_ERR, wav_file_path);
		      /* Mark that it has been deleted */
		      main_data->track[mp3_cur_track].wav_exist = FALSE;

		      /* Delete WAV work directory if this was the last WAV file,
		         and the mp3 work dir is different */
		      if (stat.tracks_remain <= 1)
			if (strcmp (config.wav_path, config.mp3_path) != 0)
			  rmdir (file_path_without_name (wav_file_path));
		    }

		  /* find next track to encode */
		  temp_track = mp3_cur_track;
		  if (find_next_job
		      (main_data, mp3_cur_track, MP3, &mp3_nxt_track,
		       &cur_type) < 0)
		    {
		      /* All finished - no more encoding - done for good! */
		      mp3_pg_pid = -1;
		      mp3_pi_pid = -1;
		      mp3_cur_track = mp3_nxt_track = -1;
		      cur_type = -1;
		      calc_stat (main_data, &stat, mp3_current,
				 temp_track, MP3, CALC_STOP_SESSION);
		      job_controller_timeout_stop ();
		      stat.encoding = FALSE;

		      job_finisher (main_data);
		      return;
		    }
		  mp3_cur_track = -1;

		  stat.tracks_done++;
		  stat.tracks_remain--;
		  stat.encoding = FALSE;

		}		/* end if job is finished section */
	      break;
	    }			/* end encode switch */
	}			/* end encode progress check */

      if (!stat.encoding && mp3_nxt_track != -1
	  && main_data->track[mp3_nxt_track].wav_exist)
	{
	  /* start encoding */
	  mp3_cur_track = mp3_nxt_track;
	  mp3_nxt_track = -1;
	  create_file_names_for_track (main_data, mp3_cur_track,
				       &wav_file_path, &enc_file_path);
	  calc_stat (main_data, &stat, mp3_current, mp3_cur_track, MP3,
		     CALC_START);
	  if (start_ripping_encoding
	      (MP3, main_data->track[mp3_cur_track].begin,
	       main_data->track[mp3_cur_track].length, mp3_cur_track,
	       wav_file_path, enc_file_path, &mp3_pg_pid, &mp3_pi_pid,
	       &mp3_read_fd) < 0)
	    {
	      calc_stat (main_data, &stat, mp3_current, mp3_cur_track, MP3,
			 CALC_STOP_SESSION);
	      job_controller_timeout_stop ();
	      job_finisher (main_data);
	    }
	  stat.encoding = TRUE;
	}

      /* end of JC_UPDATE */
      return;
    case JC_PAUSE:
      main_data = saved_main_data;

      if (wav_pg_pid >= 0)
	if (waitpid (wav_pg_pid, NULL, WNOHANG) != wav_pg_pid)
	  kill (wav_pg_pid, SIGTSTP);
      if (wav_pi_pid >= 0)
	if (waitpid (wav_pi_pid, NULL, WNOHANG) != wav_pi_pid)
	  kill (wav_pi_pid, SIGTSTP);
      if (mp3_pg_pid >= 0)
	if (waitpid (mp3_pg_pid, NULL, WNOHANG) != mp3_pg_pid)
	  kill (mp3_pg_pid, SIGTSTP);
      if (mp3_pi_pid >= 0)
	if (waitpid (mp3_pi_pid, NULL, WNOHANG) != mp3_pi_pid)
	  kill (mp3_pi_pid, SIGTSTP);

      calc_stat (main_data, &stat, wav_current, wav_cur_track, WAV,
		 CALC_PAUSE);
      job_controller_timeout_stop ();
      return;

    case JC_CONT:
      main_data = saved_main_data;

      if (wav_pg_pid >= 0)
	if (waitpid (wav_pg_pid, NULL, WNOHANG) != wav_pg_pid)
	  kill (wav_pg_pid, SIGCONT);
      if (wav_pi_pid >= 0)
	if (waitpid (wav_pi_pid, NULL, WNOHANG) != wav_pi_pid)
	  kill (wav_pi_pid, SIGCONT);
      if (mp3_pg_pid >= 0)
	if (waitpid (mp3_pg_pid, NULL, WNOHANG) != mp3_pg_pid)
	  kill (mp3_pg_pid, SIGCONT);
      if (mp3_pi_pid >= 0)
	if (waitpid (mp3_pi_pid, NULL, WNOHANG) != mp3_pi_pid)
	  kill (mp3_pi_pid, SIGCONT);

      calc_stat (main_data, &stat, wav_current, wav_cur_track, cur_type,
		 CALC_CONT);
      job_controller_timeout_start ();
      return;

    case JC_ABORT:
    case JC_ABORT_DELETE:
    case JC_ABORT_ALL:
    case JC_ABORT_ALL_DELETE:
      main_data = saved_main_data;

      if (wav_pg_pid >= 0)
	if (waitpid (wav_pg_pid, NULL, WNOHANG) != wav_pg_pid)
	  {
	    job_controller (JC_CONT, NULL);
	    kill (-wav_pg_pid, SIGTERM);
	    waitpid (wav_pg_pid, NULL, 0);
	  }
      if (wav_pi_pid >= 0)
	if (waitpid (wav_pi_pid, NULL, WNOHANG) != wav_pi_pid)
	  {
	    kill (-wav_pi_pid, SIGTERM);
	    waitpid (wav_pi_pid, NULL, 0);
	  }
      if (mp3_pg_pid >= 0)
	if (waitpid (mp3_pg_pid, NULL, WNOHANG) != mp3_pg_pid)
	  {
	    job_controller (JC_CONT, NULL);
	    kill (-mp3_pg_pid, SIGTERM);
	    waitpid (mp3_pg_pid, NULL, 0);
	  }
      if (mp3_pi_pid >= 0)
	if (waitpid (mp3_pi_pid, NULL, WNOHANG) != mp3_pi_pid)
	  {
	    kill (-mp3_pi_pid, SIGTERM);
	    waitpid (mp3_pi_pid, NULL, 0);
	  }

      /* Close the pipe or pty */
      close (wav_read_fd);
      close (mp3_read_fd);
      calc_stat (main_data, &stat, wav_current, wav_cur_track, cur_type,
		 CALC_STOP_SESSION);

      /* Destroy status widget */
      wm_status_frame_handler (WIDGET_DESTROY, WAV, &stat, msg);

      if ((ops == JC_ABORT_ALL_DELETE) || (ops == JC_ABORT_DELETE))
	{
	  create_file_names_for_track (main_data, wav_cur_track,
				       &wav_file_path, NULL);
	  if (wav_cur_track != -1)
	    {
	      create_file_names_for_track (main_data, wav_cur_track,
					   &wav_file_path, &enc_file_path);
	      if (unlink (wav_file_path) < 0)
		err_handler (GTK_WINDOW(main_window), FILE_DELETE_ERR, wav_file_path);
	    }
	  if (mp3_cur_track != -1)
	    {
	      create_file_names_for_track (main_data, mp3_cur_track, NULL,
					   &enc_file_path);
	      if (unlink (enc_file_path) < 0)
		err_handler (GTK_WINDOW(main_window), FILE_DELETE_ERR, enc_file_path);
	    }
	}
      else
	{
	  /* Mark that it exists */
	  if (wav_cur_track != -1)
	    main_data->track[wav_cur_track].wav_exist = TRUE;
	  if (mp3_cur_track != -1)
	    main_data->track[mp3_cur_track].mp3_exist = TRUE;
	}

      wav_cur_track = -1;
      mp3_cur_track = -1;
      cur_type = -1;
      calc_stat (main_data, &stat, wav_current,
		 wav_cur_track, cur_type, CALC_STOP_SESSION);
      job_controller_timeout_stop ();
      job_finisher (main_data);
      return;
    }
}
