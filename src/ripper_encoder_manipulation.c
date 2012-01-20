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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <pty.h>

#include "ripper_encoder_manipulation.h"
#include "misc_utils.h"


char **cdparanoia_create_argv (char *file_name, int track);
/* If track is >= 0, it will return a pointer to a static pointer
 * argv which will begin the ripping. If track is less than zero
 * it will return a pointer to argv which, when used in execvp, will make
 * cdparanoia to display the contents of the cd. <<IMPORTANT>> Cdparanoia
 * is supposed to ignore unnessary options given when displaying table of
 * contents */
// now only used by scan_cd !!!

int process_cd_contents_output (_main_data * main_data, int pipe_fd);

int execute_ripper_encoder_with_plugin (char *pg_com,
					char *pi_com,
					int *program_pid,
					int *plugin_pid, int *read_fd);

int parse_plugin_output (char *out, double *progress, char *msg);

int
process_cd_contents_output (_main_data * main_data, int fd)
{
  FILE *stream;
  char buf[BUF_LENGTH_FOR_F_SCAN_CD];
  char errmsg[1024];
  int i, read_offset, track_num;
  char *temp;
  int err_code = CD_PARANOIA_MISC_ERR;	// err... what should it default to?
  int cdparanoia_toc = FALSE;	// assume failure to get the table of contents

  /* Reopen the pipe as stream */
  if ((stream = fdopen (fd, "r")) == NULL)
    {
      err_handler (FDOPEN_ERR, _("Cannot reopen pty as stream"));
      return -1;
    }

  temp = fgets (buf, sizeof (buf), stream);
  /* make sure we get cdparanoia, or else there is a problem */
  if (strncmp ("cdparanoia III", buf, 14) != 0)
    {
      strcpy (errmsg, buf);
      err_handler (CD_PARANOIA_MISC_ERR, errmsg);
      return -1;
    }

  // read lines until we see "Table of contents" or another error message
  do
    {
      temp = fgets (buf, sizeof (buf), stream);
      // this is a good response
      if (strncmp ("Table of contents", temp, 17) == 0)
	{
	  cdparanoia_toc = TRUE;
	  break;
	}
      // there is no disc in the drive
      if (strncmp ("Unable to open disc.", temp, 20) == 0)
	{
	  err_code = CD_PARANOIA_NO_DISC;
	  break;
	}
      // the user does not have permissions to open the cdrom
      // (when -sv is not used)
      if (strncmp ("/dev/cdrom exists but isn\'t accessible.", temp, 39) == 0)
	{
	  err_code = CD_PARANOIA_NO_PERMS;
	  break;
	}
      // the user does not have permissions to open the cdrom
      // (when -d <device> is used) or the device does not exist
      if (strncmp ("Unable to open cdrom drive", temp, 26) == 0)
	{
	  err_code = CD_PARANOIA_MISC_ERR;
	  break;
	}
      // the user does not have permissions to open the cdrom
      // (when -sv is used)
      //     OR
      // there are no CDROM devices available
      if (strncmp ("No cdrom drives accessible to", temp, 29) == 0)
	{
	  err_code = CD_PARANOIA_MISC_ERR;
	  break;
	}
    }
  while (temp != NULL);

  /* Just to be sure, If we weren't able to read from pty it may
   * indicate exec failure */
  if (temp == NULL)
    {
      err_handler (PIPE_READ_ERR, NULL);
      return -1;
    }

  // if we didn't get a TOC, display an error message 
  if (cdparanoia_toc != TRUE)
    {
      //strncpy( errmsg, buf, BUF_LENGTH_FOR_F_SCAN_CD );
      //err_handler( err_code, errmsg );
      err_handler (err_code, NULL);
      return -1;
    }

  /* Skip two more lines */
  for (i = 0; i < 2; i++)
    fgets (buf, sizeof (buf), stream);

  /* Process the output from cdparanoia */
  track_num = 0;
  temp = fgets (buf, sizeof (buf), stream);

  while (temp != NULL && strlen (buf) > 5)
    {
      /* fix for cdparanoia 9.7 */
      if (strncmp ("TOTAL", buf, 5) != 0)
	{
	  read_offset = 0;
	  /* Skip unnecessary parts */
	  while (*(buf + read_offset++) != '.');
	  /* Read length */
	  sscanf (buf + read_offset, "%u",
		  &main_data->track[track_num].length);
	  /* Skip */
	  while (*(buf + read_offset++) != '[');

	  /* Read begin */
	  while (*(buf + read_offset++) != ']');
	  sscanf (buf + read_offset, "%u",
		  &main_data->track[track_num].begin);

	  /* Get default track title */
	  strncpy (main_data->track[track_num].title,
		   get_default_track_title (track_num), MAX_FILE_NAME_LENGTH);

	  track_num++;
	}
      temp = fgets (buf, sizeof (buf), stream);
    }

  /* Record the number of tracks */
  main_data->num_tracks = track_num;

  /* Record the total length */
  main_data->total_length =
    (int) ((main_data->track[track_num - 1].begin +
	    main_data->track[track_num - 1].length) / 75);

  /* Close the pipe */
  fclose (stream);

  return 0;
}


int
scan_cd (_main_data * main_data)
{
  pid_t pid;
  char **argv;
  char tmp[MAX_COMMAND_LENGTH];
  int null_fd, pty_fd, tty_fd;
  int return_value;

  /* Open a pty */
  if (openpty (&pty_fd, &tty_fd, NULL, NULL, NULL))
    {
      err_handler (PTY_OPEN_ERR, NULL);
      return -1;
    }

  /* Open /dev/null */
  if ((null_fd = open ("/dev/null", O_WRONLY)) < 0)
    {
      err_handler (NULL_OPEN_ERR, NULL);
      return -1;
    }

  /* Create argvs */
  sprintf (tmp, "%s -Q", config.ripper.ripper);
  if ((argv = create_argv_for_execution_using_shell (tmp)) == NULL)
    return -1;

  /* Fork */
  if ((pid = fork ()) < 0)
    {
      err_handler (FORK_ERR, NULL);
      return -1;
    }

  if (pid == 0)
    {
      int stderr_fd;

      /* This code will be excuted in the child process */
      /* Save stderr before attaching to the tty */
      stderr_fd = dup (2);
      dup2 (tty_fd, 2);

      /* Throw away stdout to the black hole */
      dup2 (null_fd, 1);

      /* Execute cdparanoia */
      execvp (argv[0], argv);

      dup2 (stderr_fd, 2);
      perror (_("Failed to exec cdparanoia :"));
      _exit (127);
    }

  close (null_fd);

  return_value = process_cd_contents_output (main_data, pty_fd);
  /* Kill again the zombie */
  waitpid (pid, NULL, 0);

  return return_value;
}

int
start_ripping_encoding (int type, int begin, int length, int track,
			char *wav_file_name, char *mp3_file_name,
			int *program_pid, int *plugin_pid, int *read_fd)
{
  char *tmp;
  char plugin[MAX_COMMAND_LENGTH];
  char command[MAX_COMMAND_LENGTH];

  // Debian modification for alternate plugin location
  // <tmancill@debian.org>
  char debian_path[MAX_COMMAND_LENGTH];
  char *path = getenv ("PATH");
  char *found = strstr (path, "/usr/lib/ripperx:");
  if (found == NULL)		/* Only add the path if it isn't already present. */
    {
      strcpy (debian_path, "/usr/lib/ripperx:");
      strcat (debian_path, getenv ("PATH"));
      setenv ("PATH", debian_path, 1);
    }
  // end Debian modifications

  // parse/expand program command
  if (type == WAV)
    {
      snprintf (command, sizeof (command), "%s %d '%s'", config.ripper.ripper,
		track + 1, wav_file_name);
    }
  else
    {
      // hack to support the interface for mp3enc
      if (!strcmp (config.encoder.encoder, "mp3enc"))
	snprintf (command, sizeof (command),
		  "nice -n %i %s -v -if '%s' -of '%s'",
		  config.encoder.priority, config.encoder.full_command,
		  wav_file_name, mp3_file_name);
      else if (!strcmp (config.encoder.encoder, "flac"))
	snprintf (command, sizeof (command), "nice -n %i %s -o '%s' '%s'",
		  config.encoder.priority, config.encoder.full_command,
		  mp3_file_name, wav_file_name);
      else if (!strcmp (config.encoder.encoder, "oggenc"))
	snprintf (command, sizeof (command), "nice -n %i %s -o '%s' '%s'",
		  config.encoder.priority, config.encoder.full_command,
		  mp3_file_name, wav_file_name);
      else
	snprintf (command, sizeof (command), "nice -n %i %s '%s' '%s'",
		  config.encoder.priority, config.encoder.full_command,
		  wav_file_name, mp3_file_name);
    }

  // parse/expand plugin command
  // plugin_executable beginning_sector length_in_sector
  if (type == WAV)
    tmp = config.ripper.plugin;
  else
    tmp = config.encoder.plugin;

  snprintf (plugin, sizeof (plugin), "%s %d %d", tmp, begin, length);

  // execute
  if (execute_ripper_encoder_with_plugin
      (command, plugin, program_pid, plugin_pid, read_fd) < 0)
    return -1;

  return 0;
}

//
//                  stdout-\ pty/tty            stdout -\ pty/tty
// ripper/encoder           ---------> plugin            ---------> ripperX
//                  stderr-/                    stderr
//
//
//
int
execute_ripper_encoder_with_plugin (char *pg_com,
				    char *pi_com,
				    int *program_pid, int *plugin_pid,
				    int *read_fd)
{
  int pty_fd0, tty_fd0, pty_fd1, tty_fd1;
  char **program_argv, **plugin_argv;
  pid_t pid;

  // build argvs
  if ((program_argv = create_argv_for_execution_using_shell (pg_com)) == NULL)
    return -1;
  if ((plugin_argv = create_argv_for_execution_using_shell (pi_com)) == NULL)
    {
      free_argv (program_argv);
      return -1;
    }

  /* Open two pty/tty pairs */
  if (openpty (&pty_fd0, &tty_fd0, NULL, NULL, NULL))
    {
      free_argv (plugin_argv);
      free_argv (program_argv);
      err_handler (PTY_OPEN_ERR, NULL);
      return -1;
    }
  if (openpty (&pty_fd1, &tty_fd1, NULL, NULL, NULL))
    {
      free_argv (plugin_argv);
      free_argv (program_argv);
      close (pty_fd0);
      close (tty_fd0);
      err_handler (PTY_OPEN_ERR, NULL);
      return -1;
    }

  // fork & exec & link plugin
  if ((pid = fork ()) < 0)
    {
      free_argv (plugin_argv);
      free_argv (program_argv);
      close (pty_fd0);
      close (tty_fd0);
      close (pty_fd1);
      close (tty_fd1);
      err_handler (FORK_ERR, NULL);
      return -1;
    }
  *plugin_pid = pid;

  if (pid == 0)
    {
      // We're in the child process
      // save stderr
      int stderr_fd;
      stderr_fd = dup (2);

      dup2 (pty_fd0, 0);
      dup2 (tty_fd1, 1);

      setpgrp ();
      execvp (plugin_argv[0], plugin_argv);

      dup2 (stderr_fd, 2);
      perror (_("Failed to exec plugin"));
      _exit (127);
    }

  // we're in the parent process
  close (pty_fd0);
  close (tty_fd1);

  // fork the real program
  if ((pid = fork ()) < 0)
    {
      free_argv (plugin_argv);
      free_argv (program_argv);
      close (tty_fd0);
      close (pty_fd1);
      kill (*plugin_pid, SIGTERM);
      err_handler (FORK_ERR, NULL);
      return -1;
    }
  *program_pid = pid;

  if (pid == 0)
    {
      // We're in the child process
      // save stderr
      int stderr_fd;
      stderr_fd = dup (2);

      dup2 (tty_fd0, 1);
      dup2 (tty_fd0, 2);

      setpgrp ();
      execvp (program_argv[0], program_argv);

      dup2 (stderr_fd, 2);
      perror (_("Failed to exec the specified program"));
      _exit (127);
    }
  close (tty_fd0);
  free_argv (plugin_argv);
  free_argv (program_argv);
  *read_fd = pty_fd1;

  return 0;
}

int
read_and_process_plugin_output (int read_fd, double *progress, char *msg)
{
  int bytes_avail;
  char buf[MAX_PLUGIN_OUTPUT_LENGTH];
  int i;

  ioctl (read_fd, FIONREAD, &bytes_avail);
  if (bytes_avail <= 0)
    // the plugin hasn't printed anything yet. return PLUGIN_NO_MSG_AVAILABLE
    return PLUGIN_NO_MSG_AVAILABLE;

  // all the lines are terminated with '\n' and if the plugin started to
  // print something then it'll finish it soon. so using fgets is
  // reasonable

  i = 0;
  do
    {
      if (read (read_fd, buf + i++, 1) <= 0)
	return PLUGIN_MSG_PARSE_ERR;
    }
  while (i < sizeof (buf) && buf[i - 1] != '\n');
  buf[i - 1] = '\n';

  return parse_plugin_output (buf, progress, msg);
}

int
parse_plugin_output (char *out, double *progress, char *msg)
{
  int pos, done, s, d;
  char ch;

  msg[0] = '\0';
  *progress = -1;

  pos = 0;
  while (out[pos] != '\0' && out[pos] != '[')
    pos++;

  if (out[pos] != '[')
    return PLUGIN_MSG_PARSE_ERR;
  pos++;

  // read the type character
  ch = out[pos++];

  if (ch == 'P')
    {
      // if it's a msg reporting progess, read the percentage
      sscanf (out + pos, "%lf", progress);
    }

  while (out[pos] != '\0' && out[pos] != '"' && out[pos] != ']')
    pos++;

  if (out[pos] == '"')
    {
      // we've got some message
      pos++;

      // copy the message
      done = 0;
      s = pos;
      d = 0;
      while (!done)
	{
	  if (out[s] != '\\' && out[s] != '"' &&
	      out[s] != ']' && out[s] != '\0')
	    msg[d++] = out[s++];
	  else if (out[s] == '\\')
	    {
	      msg[d] = out[s + 1];
	      s += 2;
	      d++;
	    }
	  else
	    {
	      msg[d] = '\0';
	      done = 1;
	    }
	}
    }

  switch (ch)
    {
    case 'P':
      if (*progress < 0 || *progress > 1)
	return PLUGIN_MSG_PARSE_ERR;
      return PLUGIN_PROGRESS_MSG;
    case 'W':
      if (msg[0] == '\0')
	return PLUGIN_MSG_PARSE_ERR;
      return PLUGIN_WARN_MSG;
    case 'E':
      if (msg[0] == '\0')
	return PLUGIN_MSG_PARSE_ERR;
      return PLUGIN_ERR_MSG;
    default:
      return PLUGIN_MSG_PARSE_ERR;
    }
}
