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
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

#include "misc_utils.h"
#include "main_window_handler.h"
#include "rw_config.h"

#include "players_manipulation.h"

char **
players_create_argv (int ops, int cd_wav_mp3, char *playit)
{
  int i, j, d;
  char buf[MAX_COMMAND_LENGTH];
  char *command = NULL;

  switch (cd_wav_mp3)
    {
    case CD:
      if (ops == PLAY)
	command = (char *) config_read (CONF_CDPL_PLAYCMD);
      else
	command = (char *) config_read (CONF_CDPL_STOPCMD);
      break;
    case WAV:
      command = (char *) config_read (CONF_WAVPL_CMD);
      break;
    case MP3:
      command = (char *) config_read (CONF_MP3PL_CMD);
      break;
    }

  // expand '%'
  strcpy (buf, command);
  for (i = 0, d = 0; command[i] != '\0';)
    if (command[i] == '%')
      {
	for (j = 0; playit[j] != '\0';)
	  buf[d++] = playit[j++];
	i++;
      }
    else
      buf[d++] = command[i++];
  buf[d] = '\0';

  return create_argv_for_execution_using_shell (buf);
}

int
play_cd_wav_mp3 (int ops, int cd_wav_mp3, char *playit)
{
  char **argv;
  static int null_fd, stderr_fd;
  pid_t pid;
  static pid_t saved_pid;
  static int is_first_time = 1;
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  if (is_first_time)
    {
      /* Open /dev/null */
      if ((null_fd = open ("/dev/null", O_WRONLY)) < 0)
	{
	  err_handler (GTK_WINDOW(main_window), NULL_OPEN_ERR, NULL);
	  return FALSE;
	}
      is_first_time = 0;
    }

  while (waitpid (-1, NULL, WNOHANG) > 0);

  /* Create appropriate argvs */
  if (ops == PLAY || cd_wav_mp3 == CD)
    {
      if ((argv = players_create_argv (ops, cd_wav_mp3, playit)) == NULL)
	return FALSE;

      /* Fork */
      if ((pid = fork ()) < 0)
	{
	  err_handler (GTK_WINDOW(main_window), FORK_ERR, NULL);
	  free_argv (argv);
	  return FALSE;
	}

      if (pid == 0)
	{
	  /* This code will be excuted in the child process */
	  /* Throw away stdout and stderr */
	  stderr_fd = dup (2);
	  dup2 (null_fd, 2);
	  dup2 (null_fd, 1);

	  /* Execute the player */
	  execvp (argv[0], argv);

	  dup2 (stderr_fd, 2);
	  perror (_("Failed to exec player :"));
	  _exit (127);
	}

      if (ops == PLAY)
	saved_pid = pid;

      free_argv (argv);
      return TRUE;
    }
  else
    {
      if (saved_pid > 0)
	if (waitpid (saved_pid, NULL, WNOHANG) == 0)
	  {
	    kill (saved_pid, SIGTERM);
	    waitpid (saved_pid, NULL, 0);
	  }
      pid = -1;
      return TRUE;
    }
}
