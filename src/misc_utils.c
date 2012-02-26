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
#include <config.h>
#endif

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <libgen.h>
#include <id3.h>
#include <glib.h>

#include "main_window_handler.h"
#include "rw_config.h"

#include "misc_utils.h"



int
add_argv (char **dest, char *content)
{
  size_t i;
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  i = 0;

  while (content[i++] != '\0');

  if ((*dest = malloc (i)) == NULL)
    {
      err_handler (GTK_WINDOW(main_window), MALLOC_ERR, NULL);
      return FALSE;
    }

  strcpy (*dest, content);
  return TRUE;
}

int
process_options (char *options, char **argv, int start, int end)
{
  int current, i, j, flag;
  char buf[MAX_SINGLE_OPTION_LENGTH];
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  current = start;
  i = 0;
  while (options[i] != '\0')
    {
      while (isspace (options[i]))
	i++;
      j = 0;
      flag = 0;
      while (!isspace (options[i])
	     && options[i] != '\0' && j < MAX_SINGLE_OPTION_LENGTH - 1)
	{
	  /* It really has something other than spaces */
	  flag = 1;
	  buf[j++] = options[i++];
	}
      buf[j] = '\0';

      /* If it really has something */
      if (flag)
	{
	  if (current < end)
	    {
	      if (add_argv (&argv[current], buf) == FALSE)
		return -1;
	    }
	  else
	    {
	      err_handler (GTK_WINDOW(main_window), TOO_MANY_ARGS_ERR, NULL);
	    }
	  current++;
	}
    }
  return current - start;
}

char **
create_argv_for_execution_using_shell (char *command)
{
  char *shell;
  char **argv;
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  shell = (char *) config_read (CONF_GNRL_SHELL_FOR_EXEC);

  if ((argv = (char **) malloc (sizeof (char *) * 4)) == NULL)
    {
      err_handler (GTK_WINDOW(main_window), MALLOC_ERR, NULL);
      return NULL;
    }
  argv[0] = NULL;
  argv[1] = NULL;
  argv[2] = NULL;
  argv[3] = NULL;

  if (add_argv (&(argv[0]), shell) == FALSE)
    return NULL;
  if (add_argv (&(argv[1]), "-c") == FALSE)
    return NULL;
  if (add_argv (&(argv[2]), command) == FALSE)
    return NULL;
  return argv;
}

void
free_argv (char **argv)
{
  int i;

  i = 0;
  while (argv[i] != NULL)
    free (argv[i++]);

  free (argv);
}

int
parse_rx_format_string (char **target, char *format, int track_no,
			char *artist, char *album, char *year, char *song)
{
  int s, d, n, pass, totlen = 0;
  char ch;
  char *tmp, *bp;
  char track_no_str[4];

  track_no_str[0] = 0;
  if (track_no >= 0)
    snprintf (track_no_str, sizeof (track_no_str), "%02d", track_no + 1);

  for (pass = 0; pass <= 1; ++pass)
    {
      for (s = d = 0; format[s] != 0; ++s)
	{
	  // determine space character and check for error
	  if (format[s] == '%')
	    {
	      ch = format[s + 1];
	      if (ch == '\0')
		{
		  return -1;
		}
	      else if (ch == '%' || ch == '#' || ch == 'a' || ch == 'v'
		       || ch == 'y' || ch == 's')
		{
		  s += 1;
		}
	      else if (ch == ' ')
		{
		  ch = format[s + 2];
		  if (ch == '\0')
		    {
		      return -1;
		    }
		  if (ch == '%' || ch == '#' || ch == 'a' || ch == 'v'
		      || ch == 'y' || ch == 's')
		    {
		      s += 2;
		    }
		}
	      else
		{
		  return -1;
		}

	      // determine what to copy
	      tmp = NULL;
	      switch (format[s])
		{
		case '%':
		  tmp = "%";
		  break;
		case '#':
		  tmp = track_no_str;
		  break;
		case 'a':
		  tmp = artist;
		  break;
		case 'v':
		  tmp = album;
		  break;
		case 'y':
		  tmp = year;
		  break;
		case 's':
		  tmp = song;
		  break;
		}
	      // expand
	      n = strlen (tmp);
	      if (!pass)
		totlen += n;
	      else
		{
		  strcpy (bp + d, tmp);
		  d += n;
		}
	    }
	  else
	    {
	      if (!pass)
		totlen += 1;
	      else
		bp[d++] = format[s];
	    }
	}
      if (!pass)
	{
	  mk_buf (target, totlen + 1);
	  bp = *target;
	}
    }
  bp[d] = '\0';
  return 0;
}

char *
time_to_readable (time_t sec)
{
  static char buf[10];

  sprintf (buf, "%2d:%02d", (int) sec / 60, (int) sec % 60);
  return buf;
}

char *
length_to_readable (unsigned length)
{
  time_t sec;

  sec = (float) length / CD_SECTORS_PER_SEC;
  return time_to_readable (sec);
}


char *
expand_tilde (char *path)
{
  static char buf[MAX_FILE_PATH_LENGTH + MAX_FILE_NAME_LENGTH];

  if (path[0] != '~')
    return path;
  strcpy (buf, getenv ("HOME"));
  strcpy (buf + strlen (buf), path + 1);
  return buf;
}

char *
construct_file_name (char *path, char *name)
{
  int offset;
  static char buf[MAX_FILE_PATH_LENGTH + MAX_FILE_NAME_LENGTH];

  strcpy (buf, path);
  offset = strlen (buf) - 1;
  if (buf[offset] != '/')
    buf[++offset] = '/';

  offset++;

  strcpy (buf + offset, name);
  return buf;
}

char *
file_name_without_path (char *src)
{
  int offset;

  offset = strlen (src) - 1;

  while (src[offset] != '/' && offset >= 0)
    offset--;

  return src + ++offset;
}

char *
file_path_without_name (char *src)
{
  static char buf[MAX_FILE_PATH_LENGTH];
  int offset;

  offset = strlen (src) - 1;

  while (src[offset] != '/' && offset >= 0)
    offset--;

  strncpy (buf, src, offset);
  buf[offset] = '\0';
  return buf;
}

void
auto_append_extension (char *src, int type)
{
  int offset;

  offset = strlen (src) - 4;
  /* aasdfasdfasd.wav */
  /*             ^    */

  if (type == WAV)
    {
      if (strcmp (".wav", src + offset) != 0)
	{
	  offset += 4;
	  strcpy (src + offset, ".wav");
	  return;
	}
    }
  else if (type == OGG)
    {
      if (strcmp (".ogg", src + offset) != 0)
	{
	  offset += 4;
	  strcpy (src + offset, ".ogg");
	  return;
	}
    }
  else if (type == FLAC)
    {
      offset--;
      if (strcmp (".flac", src + offset) != 0)
	{
	  offset += 5;
	  strcpy (src + offset, ".flac");
	  return;
	}
    }
  else if (type == MP2)
    {
      if (strcmp (".mp2", src + offset) != 0)
	{
	  offset += 4;
	  strcpy (src + offset, ".mp2");
	  return;
	}
    }
  else if (type == MUSE)
    {
      if (strcmp (".mpc", src + offset) != 0)
	{
	  offset += 4;
	  strcpy (src + offset, ".mpc");
	  return;
	}
    }
  else
    {
      if (strcmp (".mp3", src + offset) != 0)
	{
	  offset += 4;
	  strcpy (src + offset, ".mp3");
	  return;
	}
    }
}

char *
get_default_track_title (int track)
{
  static char name_buf[MAX_FILE_NAME_LENGTH];
  char *name_format = (char *) config_read (CONF_GNRL_ENC_FILEN_FORMAT);
  char track_no_buf[5];
  int read_offset, write_offset;

  read_offset = -1;
  write_offset = 0;
  while (name_format[++read_offset] != '\0')
    {
      if (name_format[read_offset] == '%')
	{
	  snprintf (track_no_buf, 5, "%d", track + 1);
	  strcpy (name_buf + write_offset, track_no_buf);
	  write_offset += strlen (track_no_buf);
	}
      else
	name_buf[write_offset++] = name_format[read_offset];
    }
  name_buf[write_offset] = '\0';
  return name_buf;
}

int
is_str_blank (char *str)
{
  int i = 0;

  if (!str)
    return TRUE;
  while (str[i++] != '\0')
    if (!isspace (str[i]) && str[i] != '\0')
      return FALSE;
  return TRUE;
}

int
high_ascii_to_low_ascii (unsigned char *p)
{
  /* Convert ASCII 160-255 to closest matching, ASCII within 32-126 */
  if (*p == 160)
    {
      *p = ' ';
      return 0;
    }
  if (*p == 161)
    {
      *p = 'i';
      return 0;
    }
  if (*p == 162)
    {
      *p = 'c';
      return 0;
    }
  if (*p == 163)
    {
      *p = 'E';
      return 0;
    }
  if (*p == 164)
    {
      *p = 'o';
      return 0;
    }
  if (*p == 165)
    {
      *p = 'Y';
      return 0;
    }
  if (*p == 166)
    {
      *p = 'I';
      return 0;
    }
  if (*p == 167)
    {
      *p = 'S';
      return 0;
    }
  if (*p == 168)
    {
      *p = '-';
      return 0;
    }
  if (*p == 169)
    {
      *p = 'c';
      return 0;
    }
  if (*p == 170)
    {
      *p = 'a';
      return 0;
    }
  if (*p == 171)
    {
      *p = '<';
      return 0;
    }
  if (*p == 172 || *p == 173)
    {
      *p = '-';
      return 0;
    }
  if (*p == 174)
    {
      *p = 'r';
      return 0;
    }
  if (*p == 175)
    {
      *p = '-';
      return 0;
    }
  if (*p == 176)
    {
      *p = 'o';
      return 0;
    }
  if (*p == 177)
    {
      *p = '+';
      return 0;
    }
  if (*p == 178)
    {
      *p = '2';
      return 0;
    }
  if (*p == 179)
    {
      *p = '3';
      return 0;
    }
  if (*p == 180)
    {
      *p = '.';
      return 0;
    }
  if (*p == 181)
    {
      *p = 'u';
      return 0;
    }
  if (*p == 182)
    {
      *p = 'P';
      return 0;
    }
  if (*p == 183 || *p == 184)
    {
      *p = '.';
      return 0;
    }
  if (*p == 185)
    {
      *p = '7';
      return 0;
    }
  if (*p == 186)
    {
      *p = 'o';
      return 0;
    }
  if (*p == 187)
    {
      *p = '>';
      return 0;
    }
  if (*p >= 188 && *p <= 190)
    {
      *p = '4';
      return 0;
    }
  if (*p == 191)
    {
      *p = '?';
      return 0;
    }
  if (*p >= 192 && *p <= 198)
    {
      *p = 'A';
      return 0;
    }
  if (*p == 199)
    {
      *p = 'C';
      return 0;
    }
  if (*p >= 200 && *p <= 203)
    {
      *p = 'E';
      return 0;
    }
  if (*p >= 204 && *p <= 207)
    {
      *p = 'I';
      return 0;
    }
  if (*p == 208)
    {
      *p = 'D';
      return 0;
    }
  if (*p == 209)
    {
      *p = 'N';
      return 0;
    }
  if (*p >= 210 && *p <= 214)
    {
      *p = 'O';
      return 0;
    }
  if (*p == 215)
    {
      *p = 'x';
      return 0;
    }
  if (*p == 216)
    {
      *p = 'O';
      return 0;
    }
  if (*p >= 217 && *p <= 220)
    {
      *p = 'U';
      return 0;
    }
  if (*p == 221)
    {
      *p = 'Y';
      return 0;
    }
  if (*p == 222)
    {
      *p = 'b';
      return 0;
    }
  if (*p == 223)
    {
      *p = 'B';
      return 0;
    }
  if (*p >= 224 && *p <= 230)
    {
      *p = 'a';
      return 0;
    }
  if (*p == 231)
    {
      *p = 'c';
      return 0;
    }
  if (*p >= 232 && *p <= 235)
    {
      *p = 'e';
      return 0;
    }
  if (*p >= 236 && *p <= 239)
    {
      *p = 'i';
      return 0;
    }
  if (*p == 240)
    {
      *p = 'o';
      return 0;
    }
  if (*p == 241)
    {
      *p = 'n';
      return 0;
    }
  if (*p >= 242 && *p <= 246)
    {
      *p = 'o';
      return 0;
    }
  if (*p == 247)
    {
      *p = '-';
      return 0;
    }
  if (*p == 248)
    {
      *p = 'o';
      return 0;
    }
  if (*p >= 249 && *p <= 252)
    {
      *p = 'u';
      return 0;
    }
  if (*p == 253)
    {
      *p = 'y';
      return 0;
    }
  if (*p == 254)
    {
      *p = 'b';
      return 0;
    }
  if (*p == 255)
    {
      *p = 'y';
      return 0;
    }

  return 1;
}

void
remove_non_unix_chars (char *src)
{
  unsigned char *p = (unsigned char *) src, *w = p;

  for (; *p != 0; p++)
    {

      // Evil chars unsafe for shell
      if (*p == '\'' || *p == '"' || *p == ';' || *p == '*')
	{
	  continue;		// Skip char, shrink string             
	}

      // Unprintable chars
      if (*p < 32 || (*p > 126 && *p < 160))
	{
	  continue;		// Skip char, shrink string
	}

      if (*p >= 160)
	{
	  high_ascii_to_low_ascii (p);
	}

      *(w++) = *p;
    }
  *w = 0;			//Terminate shrunken string
}

void
convert_slashes (char *src, char c)
{
  char *p;

  for (p = src; *p != 0; p++)
    {
      if (*p == '/')
	*p = c;

    }
}

void
convert_spaces (char *src, char c)
{
  char *p;

  for (p = src; *p != 0; p++)
    {
      if (*p == ' ')
	*p = c;

    }
}


/***************************************************
 * 
 * Function implementations immigrated from misc.c
 *
 */


char *
get_string_piece (FILE * file, int delim)
{
  /* gets one complete row from 'file' and save it in 'buffer'.
     buffer's memory will be freed and allocated to fit the stringsize
     automatically. */

  char *buffer1 = (char *) malloc (1),
    *buffer2 = (char *) malloc (1), *tmp = (char *) malloc (1024);
  char **active, **inactive;
  int i = 0;

  strcpy (buffer1, "");
  strcpy (buffer2, "");
  strcpy (tmp, "");
  do
    {
      /*switch the frames */
      if (inactive == &buffer1)
	{
	  active = &buffer1;
	  inactive = &buffer2;
	}
      else
	{
	  active = &buffer2;
	  inactive = &buffer1;
	}
      /*get the next part, and handle EOF */
      if (fgets (tmp, 1024, file) == NULL)
	{			/* ok, so we reached the end of the
				   file w/o finding the delimiter */
	  free (*active);
	  return NULL;
	}

      free (*active);
      *active = (char *) malloc ((++i) * 1024);
      sprintf (*active, "%s%s", *inactive, tmp);

    }
  while (strchr (*active, delim) == NULL);

  free (*inactive);
  return *active;
}

char *
get_ascii_file (FILE * file)
{
  /* gets one complete row from 'file' and save it in 'buffer'.
     buffer's memory will be freed and allocated to fit the stringsize
     automatically. */

  char *buffer1 = (char *) malloc (1),
    *buffer2 = (char *) malloc (1), *tmp = (char *) malloc (1024);
  char **active, **inactive;
  int i = 0;

  strcpy (buffer1, "");
  strcpy (buffer2, "");
  strcpy (tmp, "");
  do
    {
      /*switch the frames */
      if (inactive == &buffer1)
	{
	  active = &buffer1;
	  inactive = &buffer2;
	}
      else
	{
	  active = &buffer2;
	  inactive = &buffer1;
	}
      /*get the next part, and handle EOF */
      if (fgets (tmp, 1024, file) == NULL)
	{
	  free (*active);
	  return *inactive;
	}

      free (*active);
      *active = (char *) malloc ((++i) * 1024);
      sprintf (*active, "%s%s", *inactive, tmp);
    }
  while (1);
}

void
strip_trailing_space (char **string)
{
  int i = strlen (*string) - 1;
  char *return_string;

  if (string == NULL || *string == NULL)
    return;
  while (isspace ((*string)[i]))
    i--;
  i++;
  return_string = (char *) malloc (i + 1);
  strncpy (return_string, *string, i);
  return_string[i] = 0;
  free (*string);
  *string = return_string;
}

void
strip_leading_space (char **string)
{
  char *tmp = *string, *return_string;

  if (string == NULL || *string == NULL)
    return;

  while (isspace (*tmp))
    tmp++;

  return_string = strdup (tmp);
  free (*string);
  *string = return_string;
}


char *
string_append (char **dest, char *appendage)
{
  char *holder;

  if (dest == NULL || appendage == NULL)
    return NULL;

  if (*dest != NULL)
    {
      holder = *dest;
      *dest = (char *) malloc (strlen (holder) + strlen (appendage) + 1);
      sprintf (*dest, "%s%s", holder, appendage);
      free (holder);
    }
  else
    *dest = strdup (appendage);

  return *dest;
}

void
charpp_to_charp (char **dest, char **src, int num, char *separator)
{
  int i, size = 0, sep_size = 0;

  if (src == NULL || num == 0)
    return;

  if (separator == NULL)
    separator = strdup ("");

  sep_size = strlen (separator);

  for (i = 0; i < num; i++)
    size += strlen (src[i]) + sep_size;
  if (*dest != NULL)
    free (*dest);

  *dest = (char *) malloc (size + 1);
  strcpy (*dest, "");

  for (i = 0; i < num - 1; i++)
    {
      strcat (*dest, src[i]);
      strcat (*dest, separator);
    }				/* do it this way, so no separator will get in as the last part */
  strcat (*dest, src[num - 1]);

}

FILE *
socket_init (const char *server, short int port)
{
  struct hostent *host;
  struct sockaddr_in socket_address;
  int hsocket;

  /* see if some default values have been set */
  if (server == NULL)
    {
      return NULL;
    }

  host = gethostbyname (server);
  if (host == NULL)
    return NULL;		/* we don't know the host */

  /* set socket address */
  memset (&socket_address, 0, sizeof (socket_address));
  memcpy ((char *) &socket_address.sin_addr, host->h_addr, host->h_length);
  socket_address.sin_family = host->h_addrtype;
  socket_address.sin_port = htons (port);

  /* get the actual socket handle */
  hsocket = socket (host->h_addrtype, SOCK_STREAM, 0);
  if (hsocket < 0)		/* we couldn't grab the socket */
    return NULL;

  if (connect (hsocket, (struct sockaddr *) &socket_address,
	       sizeof (socket_address)) < 0)
    return NULL;

  return fdopen (hsocket, "r+");	/* we made it */
}


int
get_subdirs (const char *path, char **buffer)
{
  int hits = 0, status;
  char *subdirname;
  DIR *dirp;
  struct dirent *dir_contents;
  struct stat file_stats;

  if (path == NULL || (dirp = opendir (path)) == NULL)
    {
      return hits;
    }

  while ((dir_contents = readdir (dirp)) != NULL)
    {
      if (strcmp (dir_contents->d_name, ".") != 0 &&
	  strcmp (dir_contents->d_name, "..") != 0)
	{

	  subdirname =
	    (char *) malloc (strlen (path) + strlen (dir_contents->d_name) +
			     2);
	  sprintf (subdirname, "%s/%s", path, dir_contents->d_name);
	  status = stat (subdirname, &file_stats);

	  if (status == 0 && hits < 1000)
	    {
	      if (S_ISDIR (file_stats.st_mode))
		{		/* we are a subdir */
		  buffer[hits] = strdup (subdirname);
		  hits++;
		}
	    }
	  free (subdirname);
	}
    }

  return hits;
}

char *
int2str (int integer)
{
  char *newstr = (char *) malloc (10);
  sprintf (newstr, "%d", integer);
  return newstr;
}

long
check_free_space (char *dir)
{
  FILE *file;
  long len;
  char cmd[MAX_FILE_NAME_LENGTH];

  snprintf (cmd, MAX_FILE_NAME_LENGTH - 1,
	    "df -P '%s' 2>/dev/null | awk '{print $4}' | tail -1", dir);
  file = popen (cmd, "r");
  fscanf (file, "%ld", &len);	/* now read the length */
  pclose (file);
  return len;
}

int
check_dir (char *dir)
{
  int rc;
  struct stat ds;

  rc = access (dir, F_OK);
  if (rc != 0)
    {
      return MISC_DOES_NOT_EXISTS;
    }
  rc = stat (dir, &ds);
  if (rc != 0)
    {
      return MISC_NOT_DIR;
    }
  if (!S_ISDIR (ds.st_mode))
    {
      return MISC_NOT_DIR;
    }
  rc = access (dir, X_OK | W_OK | R_OK);
  if (rc != 0)
    {
      return MISC_NOT_WRITABLE;
    }
  return MISC_OK;
}

int
create_dir (char *path)
{
  char *parent;
  char *tmp;
  int rc;

  tmp = strdup (path);		/* preserve our string */
  parent = dirname (tmp);
  rc = check_dir (parent);
  if (strcmp (parent, "/") == 0)
    {
      return -1;		/* stop endless loop */
    }
  if (rc == MISC_NOT_DIR || rc == MISC_NOT_WRITABLE)
    {
      return -1;
    }
  else if (rc == MISC_DOES_NOT_EXISTS)
    {
      /* recursively create */
      if (create_dir (parent) != 0)
	{
	  fprintf (stderr, "failed to create %s\n", parent);
	  return -1;
	}
    }
  rc = mkdir (path, 0755);
  free (tmp);
  return rc;
}

int
is_found (char *plugin)
{
  FILE *pf;
  char buffer[MAX_COMMAND_LENGTH];
  char cmd[MAX_COMMAND_LENGTH];

  snprintf (cmd, sizeof (cmd) - 1, "%s 2>&1", plugin);

  pf = popen (cmd, "r");
  if (pf == NULL)
    {
      return 0;
    }
  fgets (buffer, sizeof (buffer) - 1, pf);
  pclose (pf);
  if (strncmp (buffer, "sh:", 3) != 0)
    {
      return 1;
    }
  return 0;
}

static char *wd, *ed;
static char *wfext, *ecfext;

int
create_filenames_from_format (_main_data * main_data)
{
  int i;
  int rc2;
  static unsigned char *df;
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);
  char *rip_path = (char *) config_read (CONF_GNRL_RIP_PATH);
  char *enc_path = (char *) config_read (CONF_GNRL_ENC_PATH);

  i = strlen (rip_path) - 1;
  if (i >= 0 && rip_path[i] == '/')
    rip_path[i] = 0;
  i = strlen (enc_path) - 1;
  if (i >= 0 && enc_path[i] == '/')
    enc_path[i] = 0;

  char *dirformat = (char *) config_read (CONF_CDDB_DIRFORMATSTR);
  char *rip_path = (char *) config_read (CONF_GNRL_RIP_PATH);
  char *enc_path = (char *) config_read (CONF_GNRL_ENC_PATH);

  if (((char *) config_read (CONF_CDDB_MKDIRS))
      && dirformat[0])
    {
      rc2 = parse_rx_format_string (&df,
				    dirformat, -1,
				    main_data->disc_artist,
				    main_data->disc_title,
				    main_data->disc_year, "");
      if (rc2 < 0)
	{
	  err_handler (GTK_WINDOW(main_window), RX_PARSING_ERR,
		       "Check if the directory format string contains format characters other than %a %# %v %y or %s.");
	  return 0;
	}

      remove_non_unix_chars (df);
      if ((int) config_read (CONF_CDDB_CONVSPC))
	{
	  convert_spaces (df, '_');
	}

      if (strlen (df) > 0)
	{
	  mk_strcat (&wd, rip_path, "/", df, "/", NULL);
	  mk_strcat (&ed, enc_path, "/", df, "/", NULL);

	  create_dir (wd);
	  create_dir (ed);
	}
      else
	{
	  mk_strcat (&wd, rip_path, "/", NULL);
	  mk_strcat (&ed, enc_path, "/", NULL);
	}

    }
  else
    {
      mk_strcat (&wd, rip_path, "/", NULL);
      mk_strcat (&ed, enc_path, "/", NULL);
    }

  if ((int) config_read (CONF_GNRL_APP_FILE_EXT))
    {
      wfext = ".wav";
      char *encoder_type = (char *) config_read (CONF_ENCOD_TYPE);
      if (encoder_type == OGG)
	ecfext = ".ogg";
      else if (encoder_type == FLAC)
	ecfext = ".flac";
      else if (encoder_type == MP2)
	ecfext = ".mp2";
      else if (encoder_type == MUSE)
	ecfext = ".mpc";
      else
	ecfext = ".mp3";
    }
  else
    wfext = ecfext = "";
  return 1;
}

int
create_file_names_for_track (_main_data * main_data, int track, char **wfp,
			     char **efp)
{
  static char *buffer;
  int rc;
  char *conv_str = NULL;
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  rc = parse_rx_format_string (&buffer,
			       ((char *) config_read (CONF_CDDB_FORMATSTR)),
			       track, main_data->disc_artist,
			       main_data->disc_title, main_data->disc_year,
			       main_data->track[track].title);
  if (rc < 0)
    {
      err_handler (GTK_WINDOW(main_window), RX_PARSING_ERR,
		   _
		   ("Check if the filename format string contains format characters other than %a %# %v %y or %s."));
      return 0;
    }

  if (buffer[0] == 0)
    strcpy (buffer, main_data->track[track].title);

  conv_str = g_locale_from_utf8 (buffer, -1, NULL, NULL, NULL);

  remove_non_unix_chars (conv_str);
  convert_slashes (conv_str, '-');
  if ((int) config_read (CONF_GNRL_CONVSPC))
    {
      convert_spaces (conv_str, '_');
    }

  if (wfp)
    mk_strcat (wfp, wd, conv_str, wfext, NULL);
  if (efp)
    mk_strcat (efp, ed, conv_str, ecfext, NULL);

  g_free (conv_str);

  return 1;
}

/*
** get_track_title - Copy track artist & title for specified track from
**                   main_data to a specified location in freedb format.
*/
void
get_track_title (char *dest, _main_data * main_data, int tno)
{
  struct _track *tk_p = &(main_data->track[tno]);
  if (tk_p->artist)
    sprintf (dest, "%s / %s", tk_p->artist, tk_p->title);
  else
    strncpy (dest, tk_p->title, MAX_FILE_NAME_LENGTH);
}

/*
** put_track_title - Put track artist & title in freedb format into
**                   main_data for specified track number.
** NOTE: A track artist can be specified
*/
void
put_track_title (char *src, _main_data * main_data, int tno)
{
  struct _track *tk_p = &(main_data->track[tno]);
  char *sp = src, *cp, *ep;
  int c;

  /* Strip leading blanks */
  while (isspace (*sp))
    ++sp;

  /* Split off track artist if specified */
  if ((cp = rindex (sp, '/')))
    {
      for (ep = cp - 1; ep > sp && isspace (*ep); --ep)
	;
      c = *(++ep);
      *ep = 0;
      mk_str (&(tk_p->artist), sp);
      *ep = c;
      for (sp = cp + 1; isspace (*sp); ++sp)
	;
    }
  else
    {
      mk_buf (&(tk_p->artist), 0);
    }

  /* Strip trailing spaces */
  for (ep = sp + strlen (sp) - 1; isspace (*ep); --ep)
    ;

  memcpy (tk_p->title, sp, ep - sp + 1);
  tk_p->title[ep - sp + 1] = 0;
}

/* dup_str - return a copy of a string in allocated memory */
char *
dup_str (char *inp)
{
  char *out;
  int len;
  if (!inp || (len = strlen (inp)) == 0)
    return (char *) 0;
  out = malloc (len + 1);
  return strcpy (out, inp);
}

/*
** mk_buf - allocate a buffer of a specified size & save the pointer.
**         If the pointer was non-null, free the storage previously
**         pointed to.
*/
void
mk_buf (char **ptr, int size)
{
  if (*ptr)
    {
      free (*ptr);
      *ptr = NULL;
    }
  if (size > 0)
    if (!(*ptr = malloc (size)))
      {
	fprintf (stderr, "Unable to allocate %d bytes of memory.. exiting\n",
		 size);
	exit (3);
      }
}

/*
** mk_str - Make a copy of a string in allocated storage saving a pointer in ptr.
**          If *ptr was non-null, free previously allocated storage.
*/
void
mk_str (char **ptr, char *inp)
{
  int n = strlen (inp);
  mk_buf (ptr, n + 1);
  if (n > 0)
    strcpy (*ptr, inp);
}

/*
** mk_strcat - Make a new string by concatenating the specified strings,
**             in order, & storing the result in allocated storage.  A
**             pointer to the result will be stored in *ptr.  The last
**			   argument must be a null pointer, marking the end of the
**             list of strings to be concatenated.
**             If *ptr is non-null, free previously allocated storage.
*/
void
mk_strcat (char **ptr, ...)
{
  va_list ap;
  char *cp, *cpb;
  int tlen = 0;

  va_start (ap, ptr);
  while ((cp = va_arg (ap, char *)))
      tlen += strlen (cp);
  va_end (ap);
  mk_buf (ptr, tlen + 1);
  va_start (ap, ptr);
  for (cpb = *ptr; (cp = va_arg (ap, char *)); cpb += strlen (cp))
    strcpy (cpb, cp);
  va_end (ap);
}


/*
 *  
 * 
 * 
 */



void
set_TagField (ID3Tag * myTag, char *data, ID3_FrameID id)
{
  ID3Frame *myFrame;
  ID3Frame *pFrame;
  char *conv_str = NULL;

  myFrame = ID3Frame_NewID (id);

  pFrame = ID3Tag_FindFrameWithID (myTag, id);

  if (pFrame != NULL)
    {
      ID3Tag_RemoveFrame (myTag, pFrame);
    }

  conv_str = g_locale_from_utf8 (data, -1, NULL, NULL, NULL);

  ID3Field_SetASCII (ID3Frame_GetField (myFrame, ID3FN_TEXT), conv_str);
  ID3Tag_AttachFrame (myTag, myFrame);

  g_free (conv_str);

  return;
}



/*
 * map a styleid onto a genre description
 */
extern char *
id3_findstyle (int styleid)
{
  if ((styleid < ID3_NR_OF_V1_GENRES) && (styleid >= 0))
    {
      return (char *) ID3_v1_genre_description[styleid];
    }
  return _("Unknown Style");
}

/*
 * map a genre description onto a styleid
 */
unsigned char
id3_find_cddb_category (char *name)
{
  int i;
  for (i = 0; i < ID3_NR_OF_V1_GENRES; i++)
    {
      if (!strcmp (ID3_v1_genre_description[i], name))
	{
	  return (unsigned char) i;
	}
    }
  return 0xFF;
}

/*
 * utility function to write vorbis tags
 */

void
vorbistag (char *ogg_file,
	   char *artist,
	   char *album,
	   char *date, char *title, unsigned char style, unsigned char track)
{
  char cmd[MAX_COMMAND_LENGTH];
  char temp[MAX_TITLE_LENGTH];
  char *tagfile;
  FILE *f;
  char *conv_artist = NULL;
  char *conv_album = NULL;
  char *conv_title = NULL;

  conv_artist = g_locale_from_utf8 (artist, -1, NULL, NULL, NULL);
  conv_album = g_locale_from_utf8 (album, -1, NULL, NULL, NULL);
  conv_title = g_locale_from_utf8 (title, -1, NULL, NULL, NULL);

  tagfile = g_strdup_printf ("%s.tags", ogg_file);

  f = fopen (tagfile, "w");
  snprintf (temp, sizeof (temp) - 1, "ARTIST=%s\n", conv_artist);
  fputs (temp, f);
  snprintf (temp, sizeof (temp) - 1, "ALBUM=%s\n", conv_album);
  fputs (temp, f);
  snprintf (temp, sizeof (temp) - 1, "DATE=%s\n", date);
  fputs (temp, f);
  snprintf (temp, sizeof (temp) - 1, "TITLE=%s\n", conv_title);
  fputs (temp, f);
  snprintf (temp, sizeof (temp) - 1, "GENRE=%s\n", id3_findstyle (style));
  fputs (temp, f);
  snprintf (temp, sizeof (temp) - 1, "TRACKNUMBER=%d\n", track);
  fputs (temp, f);
  fclose (f);

  snprintf (cmd, sizeof (cmd) - 1, "vorbiscomment -a -c '%s' '%s'", tagfile,
	    ogg_file);
  system (cmd);
  unlink (tagfile);
  free (tagfile);

  g_free (conv_artist);
  g_free (conv_album);
  g_free (conv_title);
}

// utility function to write flac tags - R. Turnbull 1-2-2010

void
flactag (char *flac_file,
	 char *artist,
	 char *album,
	 char *date, char *title, unsigned char style, unsigned char track)
{
  char cmd[MAX_COMMAND_LENGTH];
  char temp[MAX_TITLE_LENGTH];
  char *tagfile;
  FILE *f;

  tagfile = g_strdup_printf ("%s.tags", flac_file);

  f = fopen (tagfile, "w");
  snprintf (temp, sizeof (temp) - 1, "ARTIST=%s\n", artist);
  fputs (temp, f);
  snprintf (temp, sizeof (temp) - 1, "ALBUM=%s\n", album);
  fputs (temp, f);
  snprintf (temp, sizeof (temp) - 1, "DATE=%s\n", date);
  fputs (temp, f);
  snprintf (temp, sizeof (temp) - 1, "TITLE=%s\n", title);
  fputs (temp, f);
  snprintf (temp, sizeof (temp) - 1, "GENRE=%s\n", id3_findstyle (style));
  fputs (temp, f);
  snprintf (temp, sizeof (temp) - 1, "TRACKNUMBER=%d\n", track);
  fputs (temp, f);
  fclose (f);

  snprintf (cmd, sizeof (cmd) - 1, "metaflac --import-tags-from='%s' '%s'",
	    tagfile, flac_file);
  system (cmd);
  unlink (tagfile);
  free (tagfile);
}
