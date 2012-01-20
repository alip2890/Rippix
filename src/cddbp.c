/* Copyright (C) 2011
   Tony Mancill <tmancill@users.sourceforge.net>
   Dave Cinege <dcinege@psychosis.com>
   jos.dehaes@bigfoot.com

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
#include <stdio.h>
#include <string.h>

#include "cddbp.h"
#include "main_window_handler.h"

#include "misc_utils.h"

int cddbp_query_proxy (FILE *, const char *disk_id, int tracknum,
		       long int offsets[], int duration, int *matches,
		       char ***category_buffer, char ***title_buffer,
		       char ***id_buffer);
int cddbp_stat (FILE *, char **result);
int http_stat (const char *server, int port, const char *URL,
	       char **result, const char *client, const char *version);


int
cddbp_signon (FILE * socket)
{
  char *buffer;
  int code;

  buffer = get_string_piece (socket, '\n');
  if (buffer == NULL)
    return -1;
#ifdef DEBUG
  printf ("<= %s\n", buffer);
#endif
  /* convert the answer code to int */
  code =
    ((buffer[0] - 0x30) * 10 + (buffer[1] - 0x30)) * 10 + (buffer[2] - 0x30);

  free (buffer);
  return code;
}



int
cddbp_handshake (FILE * socket, const char *clientname, const char *version)
{
  char *buffer;
  int code;

  /* Greetings! */
  /* <tmancill@debian.org> - no need to disclose user and hostname */
#ifdef DEBUG
  printf ("=> cddb hello %s %s %s %s\n",
	  "anonymous", "localhost", clientname, version);
#endif
  fprintf (socket, "cddb hello %s %s %s %s\n",
	   "anonymous", "localhost", clientname, version);

  buffer = get_string_piece (socket, '\n');
  if (buffer == NULL)
    return -1;
#ifdef DEBUG
  printf ("<= %s\n", buffer);
#endif

  code =
    ((buffer[0] - 0x30) * 10 + (buffer[1] - 0x30)) * 10 + (buffer[2] - 0x30);

  free (buffer);
  return code;
}



int
cddbp_query (FILE * socket, const char *disk_id, int tracknum,
	     long int offset[], int duration, int *matches,
	     char ***category_buffer, char ***title_buffer, char ***id_buffer)
{
  char *buffer, *tmp;
  int code, i;

  buffer =
    (char *) malloc (strlen ("cddb query ") + strlen (disk_id) + 1 + 3 +
		     tracknum * 10 + 6 + 1);

  sprintf (buffer, "cddb query %s %d ", disk_id, tracknum);
  tmp = (char *) malloc (10);
  for (i = 0; i < tracknum; i++)
    {
      sprintf (tmp, "%ld ", offset[i]);
      strcat (buffer, tmp);
    }

  sprintf (tmp, "%d\n", duration);
  strcat (buffer, tmp);
  free (tmp);

#ifdef DEBUG
  printf ("=> %s\n", buffer);
#endif

  fprintf (socket, buffer);
  free (buffer);

  tmp = buffer = get_string_piece (socket, '\n');
  if (buffer == NULL)
    return -1;
#ifdef DEBUG
  printf ("<= %s\n", buffer);
#endif

  code =
    ((buffer[0] - 0x30) * 10 + (buffer[1] - 0x30)) * 10 + (buffer[2] - 0x30);
  *matches = 0;

  if (code == 200)
    {
      buffer = tmp + 4;
    }
  else if (code == 211)
    {
      free (buffer);
      tmp = buffer = get_string_piece (socket, '\n');
    }
  else
    return code;


  /* get the alternavtives */
  *title_buffer = (char **) malloc (sizeof (char *) * 1000);
  *category_buffer = (char **) malloc (sizeof (char *) * 1000);
  *id_buffer = (char **) malloc (sizeof (char *) * 1000);

  do
    {
      if (strcmp (buffer, ".\r\n") == 0)
	break;

      /* if we get more than 1000 matches, the char*[] will be full,
         and we will skip the remaining few million matches */
      if (*matches == 999)
	{
	  free (tmp);
	  continue;
	}

      i = strcspn (buffer, " ");
      (*category_buffer)[*matches] = (char *) malloc (i + 1);
      strncpy ((*category_buffer)[*matches], buffer, i);
      (*category_buffer)[*matches][i] = 0;	/*terminate the string */

      buffer += i + 1;		/* skip to the disk id */
      (*id_buffer)[*matches] = (char *) malloc (9);	/* the disk id is always
							   8 char long */
      strncpy ((*id_buffer)[*matches], buffer, 8);
      (*id_buffer)[*matches][8] = 0;	/*terminate the string */
      buffer += 9;

      i = strlen (buffer) - 2;	/*strip the remaining '\r\n' from the
				   end of the line */
      (*title_buffer)[*matches] = (char *) malloc (i + 1);
      strncpy ((*title_buffer)[*matches], buffer, i);
      (*title_buffer)[*matches][i] = 0;	/*terminate the string */

      (*matches)++;
      free (tmp);
    }
  while (code != 200 &&
	 (tmp = buffer = get_string_piece (socket, '\n')) != NULL);

  return code;
}



int
cddbp_read (FILE * socket, const char *category, const char *disk_id,
	    char **result_buffer)
{
  char *buffer;
  int code;
  int count;
  char status_buf[36];

#ifdef DEBUG
  printf ("=> cddb read %s %s\n", category, disk_id);
#endif
  fprintf (socket, "cddb read %s %s\n", category, disk_id);

  buffer = get_string_piece (socket, '\n');
  if (buffer == NULL)
    return -1;
#ifdef DEBUG
  printf ("<= %s\n", buffer);
#endif

  code =
    ((buffer[0] - 0x30) * 10 + (buffer[1] - 0x30)) * 10 + (buffer[2] - 0x30);
  free (buffer);

  if (code > 210)
    return code;
  /* skip the first row, and copy the data that follows into the result buffer */

  *result_buffer = (char *) malloc (1);

  strcpy (*result_buffer, "");
  count = 1;
  do
    {
      buffer = get_string_piece (socket, '\n');
      if (strncmp (buffer, "TTITLE", 6) == 0)
	{
	  sprintf (status_buf, _("Reading Information about track %2d"),
		   count);
	  main_window_handler (MW_UPDATE_STATUSBAR, status_buf, NULL);
	  while (gtk_events_pending ())
	    gtk_main_iteration ();
	  count++;
	}
#ifdef DEBUG
      printf ("<= %s", buffer);
#endif
      string_append (result_buffer, buffer);
      free (buffer);
    }
  while (strstr (*result_buffer, "\r\n.\r\n") == NULL);

  main_window_handler (MW_UPDATE_STATUSBAR, _("Done..."), NULL);
  while (gtk_events_pending ())
    gtk_main_iteration ();
  return code;
}

int
cddbp_stat (FILE * socket, char **result)
{
  int code;
  char *buffer;
#ifdef DEBUG
  printf ("=> stat\n");
#endif
  fprintf (socket, "stat\n");

  buffer = get_string_piece (socket, '\n');
  if (buffer == NULL)
    return -1;
#ifdef DEBUG
  printf ("<= %s\n", buffer);
#endif
  code =
    ((buffer[0] - 0x30) * 10 + (buffer[1] - 0x30)) * 10 + (buffer[2] - 0x30);
  free (buffer);

  if (code > 210)
    return code;

  do
    {
      buffer = get_string_piece (socket, '\n');
#ifdef DEBUG
      printf ("<= %s", buffer);
#endif
      string_append (result, buffer);
      free (buffer);
    }
  while (strstr (*result, "\r\n.\r\n") == NULL);

  return code;
}

void
cddbp_signoff (FILE * socket)
{
  char *buffer;
  fprintf (socket, "quit\n");
  buffer = get_string_piece (socket, '\n');
  free (buffer);
  fclose (socket);
#ifdef DEBUG
  printf ("Signing off\n");
#endif
}


int
http_query (const char *server, int port, const char *URL,
	    const char *cd_id, int tracknum, long int offset[],
	    int duration, int *matches,
	    char ***category_buffer, char ***title_buffer,
	    char ***id_buffer, const char *client, const char *version)
{
  char *buffer = (char *) malloc (1024), *tmp = (char *) malloc (1024);
  int i, code;
  FILE *sock;

  sprintf (buffer, "GET /%s?cmd=cddb+query+%s+%d", URL, cd_id, tracknum);

  for (i = 0; i < tracknum; i++)
    {
      sprintf (tmp, "+%ld", offset[i]);
      strcat (buffer, tmp);
      strcpy (tmp, "");
    }

  sprintf (tmp, "+%d&hello=%s+%s+%s+%s&proto=1 HTTP/1.0\r\n\r\n", duration,
	   "anonymous", "localhost", client, version);
  strcat (buffer, tmp);
  free (tmp);

#ifdef DEBUG
  printf ("=> %s\n", buffer);
#endif

  sock = socket_init (server, port);
  if (!sock)
    return 999;
  fprintf (sock, "%s", buffer);
  free (buffer);

  buffer = get_ascii_file (sock);
#ifdef DEBUG
  printf ("<= %s\n", buffer);
#endif

  /* strip the http header of the answer */
  tmp = strstr (buffer, "\r\n\r\n");
  if (tmp == NULL)
    {				/* uhoh, no empty row anywhere.... */
      free (buffer);
      return -1;
    }
  tmp += 4;
  tmp = strdup (tmp);
  free (buffer);
  buffer = tmp;
  *matches = 0;

  code = ((tmp[0] - 0x30) * 10 + (tmp[1] - 0x30)) * 10 + (tmp[2] - 0x30);

  if (code == 200)
    {
      buffer += 4;
    }
  else if (code == 211)
    {
      buffer = strchr (buffer, '\n');
    }
  else
    {
      free (tmp);
      return code;
    }

  *title_buffer = (char **) malloc (sizeof (char *) * 1000);
  *category_buffer = (char **) malloc (sizeof (char *) * 1000);
  *id_buffer = (char **) malloc (sizeof (char *) * 1000);

  do
    {
      if (strcmp (buffer, ".\r\n") == 0)
	break;

      /* if we get more than 1000 matches, the char*[] will be full,
         and we will skip the remaining few million matches */
      if (*matches == 999)
	{
	  continue;
	}

      i = strcspn (buffer, " ");
      if (buffer[0] == '\n')	/* skip the \n only if it is there */
	{
	  buffer++;
	  i--;
	}
      (*category_buffer)[*matches] = (char *) malloc (i + 1);
      strncpy ((*category_buffer)[*matches], buffer, i);
      (*category_buffer)[*matches][i] = 0;	/*terminate the string */

      buffer += i + 1;		/* skip to the disk id */
      (*id_buffer)[*matches] = (char *) malloc (9);	/* the disk id is always 8 char long */
      strncpy ((*id_buffer)[*matches], buffer, 8);
      (*id_buffer)[*matches][8] = 0;	/*terminate the string */
      buffer += 9;

      i = strcspn (buffer, "\r\n");
      (*title_buffer)[*matches] = (char *) malloc (i + 1);
      strncpy ((*title_buffer)[*matches], buffer, i);
      (*title_buffer)[*matches][i] = 0;	/*terminate the string */

      (*matches)++;
      buffer += i + 2;		/* skip the \r\n */
    }
  while ((buffer = strchr (buffer, '\n')) != NULL);

#ifdef DEBUG
  printf ("category: %s, title: %s", (*category_buffer)[0],
	  (*title_buffer)[0]);
#endif

  free (tmp);
  return code;
}

int
http_query_proxy (const char *server, int port, const char *proxy_server,
		  int proxy_port, const char *URL, const char *cd_id,
		  int tracknum, long int offset[], int duration, int *matches,
		  char ***category_buffer, char ***title_buffer,
		  char ***id_buffer, const char *client, const char *version)
{
  char *buffer = (char *) malloc (1024), *tmp = (char *) malloc (1024);
  int i, code;
  FILE *sock;

  sprintf (buffer, "GET http://%s:%i/%s?cmd=cddb+query+%s+%d", server, port,
	   URL, cd_id, tracknum);

  for (i = 0; i < tracknum; i++)
    {
      sprintf (tmp, "+%ld", offset[i]);
      strcat (buffer, tmp);
      strcpy (tmp, "");
    }

  sprintf (tmp, "+%d&hello=%s+%s+%s+%s&proto=1 HTTP/1.0\r\n\r\n", duration,
	   "anonymous", "localhost", client, version);
  strcat (buffer, tmp);
  free (tmp);

#ifdef DEBUG
  printf ("=> %s\n", buffer);
#endif


  sock = socket_init (proxy_server, proxy_port);
  if (!sock)
    return 999;
  fprintf (sock, "%s", buffer);
  free (buffer);

  buffer = get_ascii_file (sock);
#ifdef DEBUG
  printf ("<= %s\n", buffer);
#endif

  /* strip the http header of the answer */
  tmp = strstr (buffer, "\r\n\r\n");
  if (tmp == NULL)
    {				/* uhoh, no empty row anywhere.... */
      free (buffer);
      return -1;
    }
  tmp += 4;
  tmp = strdup (tmp);
  free (buffer);
  buffer = tmp;
  *matches = 0;

  code = ((tmp[0] - 0x30) * 10 + (tmp[1] - 0x30)) * 10 + (tmp[2] - 0x30);

  if (code == 200)
    {
      buffer += 4;
    }
  else if (code == 211)
    {
      buffer = strchr (buffer, '\n');
    }
  else
    {
      free (tmp);
      return code;
    }

  *title_buffer = (char **) malloc (sizeof (char *) * 1000);
  *category_buffer = (char **) malloc (sizeof (char *) * 1000);
  *id_buffer = (char **) malloc (sizeof (char *) * 1000);

  do
    {
      if (strcmp (buffer, ".\r\n") == 0)
	break;

      /* if we get more than 1000 matches, the char*[] will be full,
         and we will skip the remaining few million matches */
      if (*matches == 999)
	{
	  continue;
	}

      i = strcspn (buffer, " ");
      if (buffer[0] == '\n')	/* skip the \n only if it is there */
	{
	  buffer++;
	  i--;
	}
      (*category_buffer)[*matches] = (char *) malloc (i + 1);
      strncpy ((*category_buffer)[*matches], buffer, i);
      (*category_buffer)[*matches][i] = 0;	/*terminate the string */

      buffer += i + 1;		/* skip to the disk id */
      (*id_buffer)[*matches] = (char *) malloc (9);	/* the disk id is always
							   8 char long */
      strncpy ((*id_buffer)[*matches], buffer, 8);
      (*id_buffer)[*matches][8] = 0;	/*terminate the string */
      buffer += 9;

      i = strcspn (buffer, "\r\n");
      (*title_buffer)[*matches] = (char *) malloc (i + 1);
      strncpy ((*title_buffer)[*matches], buffer, i);
      (*title_buffer)[*matches][i] = 0;	/*terminate the string */

      (*matches)++;
      buffer += i + 2;		/* skip the \r\n */
    }
  while ((buffer = strchr (buffer, '\n')) != NULL);

#ifdef DEBUG
  printf ("category: %s, title: %s", (*category_buffer)[0],
	  (*title_buffer)[0]);
#endif

  free (tmp);
  return code;
}

int
http_read (const char *server, int port, const char *URL,
	   const char *category, const char *disk_id, char **result,
	   const char *client, const char *version)
{
  int code;
  char *buffer, *tmp;
  FILE *sock;

#ifdef DEBUG
  printf
    ("=> GET /%s?cmd=cddb+read+%s+%s&hello=%s+%s+%s+%s&proto=1 HTTP/1.0\r\n\r\n",
     URL, category, disk_id, "anonymouse", "localhost", client, version);
#endif

  sock = socket_init (server, port);
  if (!sock)
    return 999;

  fprintf (sock,
	   "GET /%s?cmd=cddb+read+%s+%s&hello=%s+%s+%s+%s&proto=5 HTTP/1.0\r\n\r\n",
	   URL, category, disk_id, "anonymous", "localhost", client, version);

  buffer = get_ascii_file (sock);
#ifdef DEBUG
  printf ("<= %s\n", buffer);
#endif

  /* strip the http header of the answer */
  tmp = strstr (buffer, "\r\n\r\n");
  if (tmp == NULL)
    {				/* uhoh, no empty row anywhere.... */
      free (buffer);
      return -1;
    }
  tmp += 4;
  tmp = strdup (tmp);
  free (buffer);
  buffer = tmp;

  code =
    ((buffer[0] - 0x30) * 10 + (buffer[1] - 0x30)) * 10 + (buffer[2] - 0x30);

  if (code > 210)
    {
      free (buffer);
      return code;
    }

  /* skip the first row, and copy the data that follows into the result buffer */
  tmp = strchr (buffer, '\n') + 1;
  *result = strdup (tmp);
  free (buffer);

  return code;
}


int
http_read_proxy (const char *server, int port, const char *proxy_server,
		 int proxy_port, const char *URL, const char *category,
		 const char *disk_id, char **result, const char *client,
		 const char *version)
{
  int code;
  char *buffer, *tmp;
  FILE *sock;

#ifdef DEBUG
  printf
    ("=> GET http://%s:%i/%s?cmd=cddb+read+%s+%s&hello=%s+%s+%s+%s&proto=1 HTTP/1.0\r\n\r\n",
     server, port, URL, category, disk_id, "anonymous", "localhost", client,
     version);
#endif

  sock = socket_init (proxy_server, proxy_port);
  if (!sock)
    return 999;

  fprintf (sock,
	   "GET http://%s:%i/%s?cmd=cddb+read+%s+%s&hello=%s+%s+%s+%s&proto=5 HTTP/1.0\r\n\r\n",
	   server, port, URL, category, disk_id, "anonymous", "localhost",
	   client, version);

  buffer = get_ascii_file (sock);
#ifdef DEBUG
  printf ("<= %s\n", buffer);
#endif

  /* strip the http header of the answer */
  tmp = strstr (buffer, "\r\n\r\n");
  if (tmp == NULL)
    {				/* uhoh, no empty row anywhere.... */
      free (buffer);
      return -1;
    }
  tmp += 4;
  tmp = strdup (tmp);
  free (buffer);
  buffer = tmp;

  code =
    ((buffer[0] - 0x30) * 10 + (buffer[1] - 0x30)) * 10 + (buffer[2] - 0x30);

  if (code > 210)
    {
      free (buffer);
      return code;
    }

  /* skip the first row, and copy the data that follows into the result buffer */
  tmp = strchr (buffer, '\n') + 1;
  *result = strdup (tmp);
  free (buffer);

  return code;
}


int
http_stat (const char *server, int port, const char *URL, char **result,
	   const char *client, const char *version)
{
  int code;
  char *buffer, *tmp;
  FILE *sock;

#ifdef DEBUG
  printf ("=> GET /%s?cmd=stat&hello=%s+%s+%s+%s&proto=1 HTTP/1.0\r\n\r\n",
	  URL, "anonymous", "localhost", client, version);
#endif

  sock = socket_init (server, port);
  if (!sock)
    return 999;
  fprintf (sock,
	   "GET /%s?cmd=stat&hello=%s+%s+%s+%s&proto=1 HTTP/1.0\r\n\r\n", URL,
	   "anonymous", "localhost", client, version);

  buffer = get_ascii_file (sock);
#ifdef DEBUG
  printf ("<= %s\n", buffer);
#endif

  /* strip the http header of the answer */
  tmp = strstr (buffer, "\r\n\r\n");
  if (tmp == NULL)
    {				/* uhoh, no empty row anywhere.... */
      free (buffer);
      return -1;
    }
  tmp += 4;
  tmp = strdup (tmp);
  free (buffer);
  buffer = tmp;

  code =
    ((buffer[0] - 0x30) * 10 + (buffer[1] - 0x30)) * 10 + (buffer[2] - 0x30);

  if (code > 210)
    {
      free (buffer);
      return code;
    }

  /* skip the first row, and copy the data that follows into the result buffer */
  tmp = strchr (buffer, '\n') + 1;
  *result = strdup (tmp);
  free (buffer);

  return code;
}
