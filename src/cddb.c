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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cddb.h"
#include "cddbp.h"
#include "id3.h"
#include "misc_utils.h"
#include "select_frame_handler.h"
#include "main_window_handler.h"
#include "version.h"

#define NAME "CATraxx"
#define MAX_CDDB_FILE_SIZE 15360

enum errors
{ OK, REMOTE_OK, LOCAL_OK, NOT_FOUND, NO_CONNECT, CONNECT_REFUSED,
  SERVER_ERROR, PARAM_ERROR
};
enum lookup_protocol
{ CDDBP, HTTP };
enum lookup_order
{ NONE, LOCAL, REMOTE, LOCAL_REMOTE, REMOTE_LOCAL };


int cddb_handle_data (const char *data, char **artist, char **dtitle,
		      char *titles[], int *totaltracks, char **year,
		      char **dgenre);
int do_cddb (char **result, char **disc_category, int tracknum, int duration,
	     long int offset[], const char *server, int port, int proto);


int
cddb_handle_data (const char *data, char **artist, char **dtitle,
		  char *titles[], int *totaltracks, char **year,
		  char **dgenre)
{
  char *row, *mark;
  const char *tmp = data;
  int i, j, counter = 0, track, previoustrack = 100, ttcounter = 0;
  char *tempstr;

  if (strncmp (data, "# xmcd", 6) != 0)
    return -1;

  (*dtitle) = NULL;

  while (*tmp != 0)
    {				/* check against end of string */

      /* get the row */
      i = strcspn (tmp, "\r\n");
      while (tmp[i] == '\r' || tmp[i] == '\n')
	i++;

      row = (char *) malloc (i + 1);
      strncpy (row, tmp, i);
      row[i] = 0;
      tmp += i;

      /* eval the row */
      if (strncmp (row, "DYEAR", 5) == 0)
	{			/* CD Year */
	  tempstr = malloc (MAX_YEAR_LENGTH);
	  strcpy (tempstr, row);
	  tempstr = strchr (row, '=');
	  tempstr++;
	  j = strlen (tempstr);

	  (*year) = (char *) malloc (j + 1);

	  strncpy ((*year), tempstr, (j + 1));
	  remove_non_unix_chars (*year);
	}
      else if (strncmp (row, "DGENRE", 6) == 0)
	{			/* Disc Genre */
	  tempstr = malloc (MAX_GENRE_LENGTH);
	  strcpy (tempstr, row);
	  tempstr = strchr (row, '=');
	  tempstr++;

	  j = strlen (tempstr);
	  (*dgenre) = (char *) malloc (j + 1);
	  strncpy ((*dgenre), tempstr, (j + 1));
	  remove_non_unix_chars (*dgenre);
	}
      else if (strncmp (row, "TTITLE", 6) == 0)
	{			/* Track Title */
	  /* get the track number before going on */
	  /* skip the TTITLE */
	  mark = row + 6;
	  counter = 0;

	  /* convert ascii -> int */
	  while (*mark != '=')
	    {
	      counter *= 10;
	      counter += *mark - 0x30;
	      mark++;
	    }
	  mark++;		/* and skip the '=' */
	  track = counter;

	  if (previoustrack != track)
	    ttcounter++;

	  /* create the filename. Append previous title if necessary */
	  tempstr = malloc (MAX_TITLE_LENGTH);
	  if (previoustrack == track)
	    strcpy (tempstr, titles[track]);
	  else
	    strcpy (tempstr, "");

	  /* put in the track name */
	  titles[track] = strcat (tempstr, mark);

	  strip_trailing_space (&titles[track]);
	  strip_leading_space (&titles[track]);

	  previoustrack = track;

	  remove_non_unix_chars (titles[track]);

#ifdef DEBUG
	  printf ("Track %d: %s\n", track, titles[track]);
#endif
	}
      else if ((strncmp (row, "DTITLE", 6) == 0) && (*dtitle == NULL))
	{			/* CD Title */
	  i = strcspn (row, "=");
	  i++;			/* skip to the data */
	  mark = row + i;

	  // tm:  hack around bogus CDDB entries
	  if (strstr (mark, " / "))
	    {
	      j = strstr (mark, " / ") - mark + 1;
	      (*artist) = (char *) malloc (j + 1);
	      strncpy ((*artist), mark, j);
	      (*artist)[j] = 0;
	      (*dtitle) = (char *) malloc (strlen (mark) - j);
	      mark = mark + j + 1;
	      strcpy ((*dtitle), mark);
	    }
	  else
	    {
#ifdef DEBUG
	      printf
		("malformed DTITLE, copying full DTITLE into both artist and dtitle\n");
#endif
	      j = strlen (mark);
	      (*artist) = (char *) malloc (j + 1);
	      strncpy ((*artist), mark, j);
	      (*artist)[j] = 0;
	      (*dtitle) = (char *) malloc (j + 1);
	      strncpy ((*dtitle), mark, j);
	      (*dtitle)[j] = 0;
	    }

	  strip_trailing_space (artist);
	  strip_leading_space (artist);
	  strip_trailing_space (dtitle);
	  strip_leading_space (dtitle);

	  remove_non_unix_chars (*artist);
	  remove_non_unix_chars (*dtitle);
	  convert_slashes (*artist, '_');	// dc: _ Artist, - others
	  convert_slashes (*dtitle, '-');
#ifdef CONVERT_SPACES_IN_ID3_DATA
	  if (config.cddb_config.convert_spaces == TRUE)
	    {
	      convert_spaces (*artist, '_');
	      convert_spaces (*dtitle, '_');
	    }
#endif
#ifdef DEBUG
	  printf ("Artist: %s\n", (*artist));
	  printf ("Dtitle: %s\n", (*dtitle));
#endif
	}

      /* ignore any other results */
      free (row);
    }

  *totaltracks = ttcounter;
  return 0;
}


unsigned long int
cddb_disk_id (int length, int tracknum, long int offset[])
{
  int track;
  long int result = 0, tmp;

  for (track = 0; track < tracknum; track++)
    {
      tmp = (offset[track] + 150) / 75;
      do
	{
	  result += tmp % 10;
	  tmp /= 10;
	}
      while (tmp != 0);
    }

  return (result % 0xff) << 24 | (length + 2 -
				  ((offset[0] + 150) / 75)) << 8 | tracknum;
}

int
do_cddb (char **result, char **disc_category, int tracknum, int duration,
	 long int offset[], const char *server, int port, int proto)
{
  FILE *sock = NULL;
  int status, matches, i;
  char **category = NULL, **title = NULL, **alt_id = NULL;

  char *final_cat, *final_id, *cd_id, *uri, *wwwserver;

  /* chop up the URL */
  wwwserver = uri = strdup (server);
  wwwserver = strsep (&uri, "/");

  final_cat = (char *) malloc (20);
  final_id = (char *) malloc (20);
  cd_id = (char *) malloc (20);
  sprintf (cd_id, "%08lx", cddb_disk_id (duration, tracknum, offset));

  if (proto == CDDBP)
    {
      sock = socket_init (wwwserver, port);
      if (!sock)
	return NO_CONNECT;

      /* connect to the cddb server */
      status = cddbp_signon (sock);
      switch (status)
	{
	case 200:
	case 201:
	  break;		/* might want to differenciate when write() is supported on the server side */
	case 432:
	case 433:
	case 434:
	  fclose (sock);
	  return CONNECT_REFUSED;
	default:
	  fclose (sock);
	  return SERVER_ERROR;
	}

      /* do the hello handshake */
      status = cddbp_handshake (sock, NAME, VERSION);
      switch (status)
	{
	case 200:
	case 402:
	  break;
	case 431:
	  fclose (sock);
	  return CONNECT_REFUSED;
	default:
	  fclose (sock);
	  return SERVER_ERROR;
	}

      status = cddbp_query (sock, cd_id, tracknum, offset, duration, &matches,
			    &category, &title, &alt_id);
    }

#if 0
  /* get cddb information from a local http server (plain text file) */
  /* must change CDDB URL to be http://localhost/testentry.txt */
  strcpy (final_cat, "test");
  strcpy (final_id, "test");
#else
  else
    {
      if (!strcmp (config.cddb_config.proxy_server, ""))
	status =
	  http_query (wwwserver, port, uri, cd_id, tracknum, offset, duration,
		      &matches, &category, &title, &alt_id, NAME, VERSION);
      else
	status =
	  http_query_proxy (wwwserver, port, config.cddb_config.proxy_server,
			    config.cddb_config.proxy_port, uri, cd_id,
			    tracknum, offset, duration, &matches, &category,
			    &title, &alt_id, NAME, VERSION);

    }

  switch (status)
    {
    case 200:
    case 211:
      break;
    case 999:
      return NO_CONNECT;
    case 202:
      if (proto == CDDBP)
	cddbp_signoff (sock);
      return NOT_FOUND;
    case 403:
      if (proto == CDDBP)
	cddbp_signoff (sock);
      return SERVER_ERROR;
    default:
      if (proto == CDDBP)
	fclose (sock);
      return SERVER_ERROR;
    }

  /* we don't care about multiple matches, just grab the first one */
  final_cat = strdup (category[0]);
  *disc_category = final_cat;
  final_id = strdup (alt_id[0]);

  for (i = 0; i < matches; i++)
    {
      free (category[i]);
      free (title[i]);
      free (alt_id[i]);
    }
  free (category);
  free (title);
  free (alt_id);
#endif

  /* now finally grab the data for the disc id and category */
  if (proto == CDDBP)
    status = cddbp_read (sock, final_cat, final_id, result);
  else if (!strcmp (config.cddb_config.proxy_server, ""))
    status =
      http_read (wwwserver, port, uri, final_cat, final_id, result, NAME,
		 VERSION);
  else
    status =
      http_read_proxy (wwwserver, port, config.cddb_config.proxy_server,
		       config.cddb_config.proxy_port, uri, final_cat,
		       final_id, result, NAME, VERSION);

  switch (status)
    {
    case 210:
      break;
    case 401:
      if (proto == CDDBP)
	cddbp_signoff (sock);
      return NOT_FOUND;
    case 403:
      if (proto == CDDBP)
	cddbp_signoff (sock);
      return SERVER_ERROR;
    default:
      if (proto == CDDBP)
	fclose (sock);
      return SERVER_ERROR;
    }

  if (proto == CDDBP)
    cddbp_signoff (sock);

  free (final_id);

  return REMOTE_OK;
}

/*
	  check the local file system to see if the file
	  already exists by looping thru the catagories and the 
	  predefined prefix
*/
int
read_local_file (char **result, int tracknum, int duration, long int offset[])
{
  FILE *datafile;
  int i;
  char file_check[MAX_FILE_PATH_LENGTH + MAX_FILE_NAME_LENGTH + 1];
  char *cd_id;

  *result = (char *) malloc (MAX_CDDB_FILE_SIZE);
  cd_id = (char *) malloc (20);
  sprintf (cd_id, "%08lx", cddb_disk_id (duration, tracknum, offset));
#ifdef DEBUG
  printf ("cddb_disk_id returned '%s'\n", cd_id);
#endif
  /* first, check the directory pointed to by the config file */
  strcpy (file_check, config.cddb_path);	/* start with the base path */
  strcat (file_check, "/");
  strcat (file_check, cd_id);	/* add the filename */
  if ((datafile = fopen (file_check, "r")) == NULL)
    {
      /*  
       *  since the file didn't exist in the directory
       *      check the category sub-directories
       */
      for (i = 0; i < ID3_NR_OF_V1_GENRES; i++)
	{

	  strcpy (file_check, config.cddb_path);	/*   start with the base path  */
	  strcat (file_check, "/");	/*   add the seperator  */
	  strcat (file_check, ID3_v1_genre_description[i]);	/*   add the catagory  */
	  strcat (file_check, "/");	/*   add the seperator  */
	  strcat (file_check, cd_id);	/*   add the filename  */
	  if ((datafile = fopen (file_check, "r")) != NULL)
	    break;
	}
    }
  if (datafile == NULL)
    return (NOT_FOUND);
  else
    {
      fread (*result, 1, MAX_CDDB_FILE_SIZE, datafile);
      fclose (datafile);
      return (REMOTE_OK);
    }
}

int
do_cddb_proxy (char **result, char **disc_category, int tracknum,
	       int duration, long int offset[], const char *server, int port,
	       int proto)
{
  FILE *sock = NULL;
  int status, matches, i;
  char **category = NULL, **title = NULL, **alt_id = NULL;
  char *final_cat, *final_id, *cd_id, *uri, *wwwserver;

  /* chop up the URL */
  wwwserver = uri = strdup (server);
  wwwserver = strsep (&uri, "/");

  final_cat = (char *) malloc (20);
  final_id = (char *) malloc (20);
  cd_id = (char *) malloc (20);

  sprintf (cd_id, "%08lx", cddb_disk_id (duration, tracknum, offset));
  if (proto == CDDBP)
    {
      sock = socket_init (wwwserver, port);
      if (!sock)
	return NO_CONNECT;

      /* connect to the cddb server */
      status = cddbp_signon (sock);
      switch (status)
	{
	case 200:
	case 201:
	  break;		/* might want to differentiate when write() is supported on the server side */
	case 432:
	case 433:
	case 434:
	  fclose (sock);
	  return CONNECT_REFUSED;
	default:
	  fclose (sock);
	  return SERVER_ERROR;
	}

      /* do the hello handshake */
      status = cddbp_handshake (sock, NAME, VERSION);
      switch (status)
	{
	case 200:
	case 402:
	  break;
	case 431:
	  fclose (sock);
	  return CONNECT_REFUSED;
	default:
	  fclose (sock);
	  return SERVER_ERROR;
	}

      status = cddbp_query (sock, cd_id, tracknum, offset, duration, &matches,
			    &category, &title, &alt_id);
    }

#ifdef TESTLOCALHTTP
  /* get cddb information from a local http server (plain text file) */
  /* must change CDDB URL to be http://localhost/testentry.txt */
  strcpy (final_cat, "test");
  strcpy (final_id, "test");
#else
  else
    {
      status =
	http_query (wwwserver, port, uri, cd_id, tracknum, offset, duration,
		    &matches, &category, &title, &alt_id, NAME, VERSION);
    }

  switch (status)
    {
    case 200:
    case 211:
      break;
    case 999:
      return NO_CONNECT;
    case 202:
      if (proto == CDDBP)
	cddbp_signoff (sock);
      return NOT_FOUND;
    case 403:
      if (proto == CDDBP)
	cddbp_signoff (sock);
      return SERVER_ERROR;
    default:
      if (proto == CDDBP)
	fclose (sock);
      return SERVER_ERROR;
    }

  /* we don't care about multiple matches, just grab the first one */
  final_cat = strdup (category[0]);
  *disc_category = final_cat;
  final_id = strdup (alt_id[0]);

  for (i = 0; i < matches; i++)
    {
      free (category[i]);
      free (title[i]);
      free (alt_id[i]);
    }
  free (category);
  free (title);
  free (alt_id);
#endif

  /* now finally grab the data for the disc id and category */
  if (proto == CDDBP)
    status = cddbp_read (sock, final_cat, final_id, result);
  else
    status = http_read (wwwserver, port, uri, final_cat, final_id, result,
			NAME, VERSION);

  switch (status)
    {
    case 210:
      break;
    case 401:
      if (proto == CDDBP)
	cddbp_signoff (sock);
      return NOT_FOUND;
    case 403:
      if (proto == CDDBP)
	cddbp_signoff (sock);
      return SERVER_ERROR;
    default:
      if (proto == CDDBP)
	fclose (sock);
      return SERVER_ERROR;
    }

  if (proto == CDDBP)
    cddbp_signoff (sock);

  free (final_id);

  return REMOTE_OK;
}

int
cddb_main (_main_data * main_data)
{
  int err, i, totaltracks;

  char *result = NULL;
  char *artist = NULL;
  char *dtitle = NULL;
  char *category = NULL;
  char *titles[MAX_NUM_TRACK];
  char *year = NULL;
  char *dgenre = NULL;

  /* Main window is needed for error message dialog */
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  int tracknum = main_data->num_tracks;
  int duration = main_data->total_length;
  long int offset[MAX_NUM_TRACK];

  main_window_handler (MW_UPDATE_STATUSBAR, _("Contacting CDDB server..."),
		       NULL);
  while (gtk_events_pending ())
    gtk_main_iteration ();

  /* grab the track offsets from the main_data structure */
  for (i = 0; i < tracknum; i++)
    offset[i] = main_data->track[i].begin;

  /* check for a local file first */
  err = read_local_file (&result, tracknum, duration, offset);

  /* connect to the cddb server and grab the results */
  if (err != REMOTE_OK)
    err =
      do_cddb (&result, &category, tracknum, duration, offset,
	       config.cddb_config.server, config.cddb_config.port,
	       config.cddb_config.use_http);

  main_window_handler (MW_UPDATE_STATUSBAR, _("Grabbing Completed..."), NULL);
  while (gtk_events_pending ())
    gtk_main_iteration ();

  switch (err)
    {
    case REMOTE_OK:
      /* successful lookup, now parse the returned data */
      cddb_handle_data (result, &artist, &dtitle, titles, &totaltracks, &year,
			&dgenre);

      if (artist == 0)
	strcpy (main_data->disc_artist, "");
      else
	strcpy (main_data->disc_artist, artist);
      free (artist);
      if (dtitle == 0)
	strcpy (main_data->disc_title, "");
      else
	strcpy (main_data->disc_title, dtitle);
      free (dtitle);
      if (dgenre == 0)
	strcpy (main_data->disc_category, "");
      else
	strcpy (main_data->disc_category, dgenre);
      free (category);
      if (year == 0)
	strcpy (main_data->disc_year, "");
      else
	strcpy (main_data->disc_year, year);
      free (year);
      for (i = 0; i < tracknum; i++)	/* fixed a sneaky bug here */
	{
	  /* copy the data into the main_data structure */
	  put_track_title (titles[i], main_data, i);
	  free (titles[i]);
	}

      // auto-select all tracks
      select_frame_handler (SF_SELECT_ALL, 0, main_data);
      /* this is needed to update the display with the new data */
      select_frame_handler (SF_SYNC_SELECT_FRAME, 0, main_data);

      break;
    case NO_CONNECT:
      err_handler (GTK_WINDOW(main_window), CDDB_NO_CONNECT_ERR, NULL);
      break;
    case CONNECT_REFUSED:
      err_handler (GTK_WINDOW(main_window), CDDB_CONNECT_REFUSED_ERR, NULL);
      break;
    case SERVER_ERROR:
      err_handler (GTK_WINDOW(main_window), CDDB_SERVER_ERR, NULL);
      break;
    case NOT_FOUND:
      err_handler (GTK_WINDOW(main_window), CDDB_NOT_FOUND_ERR, NULL);
      break;
    }

  main_window_handler (MW_CLEAR_STATUSBAR, NULL, NULL);
  while (gtk_events_pending ())
    gtk_main_iteration ();

  return 0;
}
