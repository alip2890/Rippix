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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "misc_utils.h"
#include "interface_common.h"
#include "version.h"
#include "main_window_handler.h"

#include "config_rw.h"

void config_to_default (int item_num);
/* -1 is all, otherwise the item_num th field */
void read_an_item (int item_num, char *src);

#define STRING                  0
#define CHAR                    1
#define FLOAT                   2
#define INT                     3

#define MAX_CONFIG_LINE_LENGTH  300

/* Data used by read write config functions */
static struct
{
  char *f_id;
  void *m_id;
  int type;
  int flag;			/* Flag is for error checking when reading config file */
  char *default_value;
} config_rw_data[] =
{
  {
  "General::WavRatio", &config.wav_ratio, FLOAT, 0, "0.006"},
  {
  "General::Mp3Ratio", &config.mp3_ratio, FLOAT, 0, "0.08"},
  {
  "General::ShellForExecution", config.shell_for_execution,
      STRING, 0, "/bin/sh"},
  {
  "General::WavPath", config.wav_path, STRING, 0, "./"},
  {
  "General::Mp3Path", config.mp3_path, STRING, 0, "./"},
  {
  "General::CDDBPath", config.cddb_path, STRING, 0, "./.cddbslave"},
  {
  "General::WavFileNameFormat", config.wav_file_name_format,
      STRING, 0, "track%"},
  {
  "General::Mp3FileNameFormat", config.mp3_file_name_format,
      STRING, 0, "track%"},
  {
  "General::PrependChar", &config.prepend_char, CHAR, 0, "_"},
  {
  "General::MakeMp3FromExistingWav", &config.make_mp3_from_existing_wav,
      INT, 0, "0"},
  {
  "General::AskWhenFileExists", &config.ask_when_file_exists, INT, 0, "1"},
  {
  "General::AutoAppendExtension", &config.auto_append_extension, INT, 0, "1"},
  {
  "General::KeepWav", &config.keep_wav, INT, 0, "0"},
  {
  "Ripper::Ripper", config.ripper.ripper,
      STRING, 0, "cdparanoia                "},
  {
  "Ripper::Plugin", config.ripper.plugin,
      STRING, 0, "ripperX_plugin-cdparanoia"},
  {
  "Encoder::Encoder", config.encoder.encoder, STRING, 0, "lame"},
  {
  "Encoder::Type", &config.encoder.type, INT, 0, "2"},
  {
  "Encoder::Bitrate", &config.encoder.bitrate, INT, 0, "128"},
  {
  "Encoder::VarBitrate", &config.encoder.use_varbitrate, INT, 0, "1"},
  {
  "Encoder::VBRQual", &config.encoder.vbr_qual, INT, 0, "4"},
  {
  "Encoder::Priority", &config.encoder.priority, INT, 0, "10"},
  {
  "Encoder::HighQual", &config.encoder.use_high_qual, INT, 0, "1"},
  {
  "Encoder::useCRC", &config.encoder.use_crc, INT, 0, "0"},
  {
  "Encoder::extraOptions", config.encoder.extra_options, STRING, 0, ""},
  {
  "Encoder::fullCommand", config.encoder.full_command,
      STRING, 0, "lame -b 128"},
  {
  "Encoder::Plugin", config.encoder.plugin, STRING, 0, "ripperX_plugin-lame"},
  {
  "CdPlayer::Play_command", config.cd_player.play_command,
      STRING, 0, "cdplay %"},
  {
  "CdPlayer::Stop_command", config.cd_player.stop_command,
      STRING, 0, "cdstop"},
  {
  "WavPlayer::Command", config.wav_player.command, STRING, 0, "play %"},
  {
  "Mp3Player::Command", config.mp3_player.command, STRING, 0, "mpg123 %"},
  {
  "CDDBConfig::Server", config.cddb_config.server,
      STRING, 0, "freedb.freedb.org/~cddb/cddb.cgi"},
  {
  "CDDBConfig::Port", &config.cddb_config.port, INT, 0, "80"},
  {
  "CDDBConfig::UseHttp", &config.cddb_config.use_http, INT, 0, "1"},
  {
  "CDDBConfig::ProxyServer", &config.cddb_config.proxy_server, STRING, 0, ""},
  {
  "CDDBConfig::ProxyPort", &config.cddb_config.proxy_port, INT, 0, "8080"},
  {
  "CDDBConfig::ConvertSpaces", &config.cddb_config.convert_spaces,
      INT, 0, "0"},
  {
  "CDDBConfig::MakeDirectories", &config.cddb_config.make_directories,
      INT, 0, "1"},
  {
  "CDDBConfig::CreateID3", &config.cddb_config.create_id3, INT, 0, "1"},
  {
  "CDDBConfig::CreatePlaylist", &config.cddb_config.create_playlist,
      INT, 0, "1"},
  {
  "CDDBConfig::AutoLookup", &config.cddb_config.auto_lookup, INT, 0, "0"},
  {
  "CDDBConfig::FormatString", &config.cddb_config.format_string,
      STRING, 0, "%a - %s"},
  {
  "CDDBConfig::DirFormatString", &config.cddb_config.dir_format_string,
      STRING, 0, "%a - %v"}
};

static int num_entry = sizeof (config_rw_data) / sizeof (config_rw_data[0]);

void
read_an_item (int item_num, char *src)
{
  char *str;
  char *p_char;
  float *p_float;
  int *p_int;
  int i, len;
  GtkWindow *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  switch (config_rw_data[item_num].type)
    {
    case STRING:
      str = (char *) config_rw_data[item_num].m_id;
      strcpy (str, src);
      /* Strip \n */
      len = strlen (str);
      for (i = 0; i < len && str[i] != '\0'; i++)
	if (str[i] == '\n')
	  str[i--] = '\0';
      break;

    case CHAR:
      p_char = (char *) config_rw_data[item_num].m_id;
      *p_char = src[0];
      break;

    case FLOAT:
      p_float = (float *) config_rw_data[item_num].m_id;
      sscanf (src, "%f", p_float);
      break;

    case INT:
      p_int = (int *) config_rw_data[item_num].m_id;
      sscanf (src, "%d", p_int);
      break;

    default:
      err_handler (GTK_WINDOW(main_window), CONFIG_PARSE_ERR, NULL);
      break;
    }
}

void
config_to_default (int item_num)
{
  int i;

  if (item_num < 0)
    {
      for (i = 0; i < num_entry; i++)
	read_an_item (i, config_rw_data[i].default_value);
      return;
    }

  read_an_item (item_num, config_rw_data[item_num].default_value);
}

void
write_config (void)
{
  FILE *file;
  int fd;
  int i;
  char *str;
  char t_char;
  float t_float;
  int t_int;
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  fd = open (construct_file_name (getenv ("HOME"), ".ripperXrc"),
	     O_WRONLY | O_TRUNC);

  if (fd < 0)
    {
      if (errno == ENOENT)
	{
	  if (dialog_handler (WIDGET_CREATE, FALSE, DL_CREATE_CONFIG_CONFIRM,
			      FALSE, NULL, NULL, 0) == FALSE)
	    return;
	  fd = open (construct_file_name (getenv ("HOME"), ".ripperXrc"),
		     O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	  if (fd < 0)
	    {
	      err_handler (GTK_WINDOW(main_window), CONFIG_CREATION_ERR, "");
	      return;
	    }
	}
      else
	err_handler (GTK_WINDOW(main_window), CONFIG_OPEN_ERR, "");
    }

  if ((file = fdopen (fd, "w")) == NULL)
    {
      err_handler (GTK_WINDOW(main_window), FDOPEN_ERR, "Cannot re-open config file as a stream");
      close (fd);
      return;
    }

  fputs ("//\n", file);
  fputs ("// ~/.ripperXrc\n", file);
  fputs ("// This is the resource file for ripperX.\n", file);
  fputs ("// If you edit this file with an editor, you must leave all\n",
	 file);
  fputs ("// parameters in the order in which they appear.  Also note\n",
	 file);
  fputs ("// that this file is overwritten each time ripperX is run.\n",
	 file);
  fputs
    ("//\n// You can configure everything in the config menu within ripperX.\n",
     file);
  fputs ("//\n\n", file);

  fprintf (file, "//-V %s\n\n", VERSION);

  for (i = 0; i < num_entry; i++)
    {
      switch (config_rw_data[i].type)
	{
	case STRING:
	  str = (char *) config_rw_data[i].m_id;

	  fprintf (file, "%s = %s\n", config_rw_data[i].f_id, str);
	  break;

	case CHAR:
	  t_char = *(char *) config_rw_data[i].m_id;

	  fprintf (file, "%s = %c\n", config_rw_data[i].f_id, t_char);
	  break;

	case FLOAT:
	  t_float = *(float *) config_rw_data[i].m_id;

	  fprintf (file, "%s = %f\n", config_rw_data[i].f_id, t_float);
	  break;

	case INT:
	  t_int = *(int *) config_rw_data[i].m_id;

	  fprintf (file, "%s = %d\n", config_rw_data[i].f_id, t_int);
	  break;
	}
    }
  fclose (file);
  return;
}

void
read_config (void)
{
  FILE *file;
  char buf[MAX_CONFIG_LINE_LENGTH + 1];
  int i, offset;
  int flag;
  GtkWidget *main_window = main_window_handler(MW_REQUEST_MW, NULL, NULL);

  memset (&config, 0, sizeof (_config));

  file = fopen (construct_file_name (getenv ("HOME"), ".ripperXrc"), "r");

  if (file == NULL)
    {
      config_to_default (-1);
      if (errno != ENOENT)
	{
	  err_handler (GTK_WINDOW(main_window), CONFIG_OPEN_ERR, NULL);
	  return;
	}
      write_config ();
      return;
    }

  while (fgets (buf, sizeof (buf), file) != NULL)
    {
      /* Comments */
      if (strncasecmp ("//", buf, 2) == 0)
	continue;
      for (i = 0, offset = 0; i < num_entry && offset == 0; i++)
	{
	  /* Find the match */
	  if (strncasecmp (config_rw_data[i].f_id, buf,
			   strlen (config_rw_data[i].f_id)) == 0)
	    {
	      /* General::WavRatio = asdf... */
	      offset += strlen (config_rw_data[i].f_id);
	      while (buf[offset++] != '=');
	    }
	}
      i--;

      /* Mark that this field has been read */
      config_rw_data[i].flag = TRUE;

      /* Skip blank lines */
      if (is_str_blank (buf + offset))
	continue;

      /* Skip spaces */
      while (isspace (buf[offset++]));
      offset--;

      read_an_item (i, buf + offset);
    }

  for (i = 0, flag = FALSE; i < num_entry; i++)
    {
      if (config_rw_data[i].flag == FALSE)
	{
	  flag = TRUE;
	  config_to_default (i);
	}
    }

  if (flag)
    err_handler (GTK_WINDOW(main_window), CONFIG_EMPTY_ITEM_ERR, "");

  return;
}
