/* Copyright (C) 2012
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

/* This file contains function declarations for querying the GConf database
   in a comfortably way.  This includes functions for reading and writing
   to the database. */

#ifndef RW_CONFIG_H
#define RW_CONFIG_H

#define _GNU_SOURCE

/* Use these macros to pass a key to reading / writing functions.
   See share/rippix.schemas for explanation of these keys.
 */
#define CONF_GNRL_RIP_RATIO "/apps/rippix/general/rip_ratio"
#define CONF_GNRL_ENC_RATIO "/apps/rippix/general/encode_ratio"
#define CONF_GNRL_SHELL_FOR_EXEC "/apps/rippix/general/shell_for_execution"
#define CONF_GNRL_RIP_PATH "/apps/rippix/general/rip_path"
#define CONF_GNRL_ENC_PATH "/apps/rippix/general/encode_path"
#define CONF_GNRL_CDDB_PATH "/apps/rippix/general/cddb_path"
#define CONF_GNRL_ENC_FILEN_FORMAT "/apps/rippix/general/encoded_filename_format"
#define CONF_GNRL_WAV_FILEN_FORMAT "/apps/rippix/general/wav_filename_format"
#define CONF_GNRL_PREP_CHAR "/apps/rippix/general/prepend_char"
#define CONF_GNRL_ENC_FROM_EXIST "/apps/rippix/general/encode_from_existing_files"
#define CONF_GNRL_ASK_FILE_EXIST "/apps/rippix/general/ask_when_file_exists"
#define CONF_GNRL_APP_FILE_EXT "/apps/rippix/general/autoappend_file_extension"
#define CONF_GNRL_KEEP_WAV "/apps/rippix/general/keep_wav"
#define CONF_RPR_RIPPER "/apps/rippix/ripper/ripper"
#define CONF_RPR_PLUGIN "/apps/rippix/ripper/plugin"
#define CONF_ENCOD_ENCODER "/apps/rippix/encoder/encoder"
#define CONF_ENCOD_TYPE "/apps/rippix/encoder/type"
#define CONF_ENCOD_BITRATE "/apps/rippix/encoder/bitrate"
#define CONF_ENCOD_VARBITRATE "/schemas/apps/rippix/encoder/var_bitrate"
#define CONF_ENCOD_VBRQUAL "/apps/rippix/encoder/vbr_quality"
#define CONF_ENCOD_PRIORITY "/apps/rippix/encoder/priority"
#define CONF_ENCOD_HIGHQUAL "/apps/rippix/encoder/high_quality"
#define CONF_ENCOD_USECRC "/apps/rippix/encoder/use_crc"
#define CONF_ENCOD_EXTRAOPTS "/apps/rippix/encoder/extra_options"
#define CONF_ENCOD_FULLCMD "/apps/rippix/encoder/full_command"
#define CONF_ENCOD_PLUGIN "/apps/rippix/encoder/plugin"
#define CONF_CDPL_PLAYCMD "/apps/rippix/cdplayer/play_command"
#define CONF_CDPL_STOPCMD "/apps/rippix/cdplayer/stop_command"
#define CONF_WAVPL_CMD "/apps/rippix/wavplayer/command"
#define CONF_MP3PL_CMD "/apps/rippix/mp3player/command"
#define CONF_CDDB_SERVER "/apps/rippix/cddb/server"
#define CONF_CDDB_PORT "/apps/rippix/cddb/port"
#define CONF_CDDB_USEHTTP "/apps/rippix/cddb/use_http"
#define CONF_CDDB_PROXYSRV "/apps/rippix/cddb/proxy_server"
#define CONF_CDDB_PROXYPRT "/apps/rippix/cddb/proxy_port"
#define CONF_CDDB_CONVSPC "/apps/rippix/cddb/convert_spaces"
#define CONF_CDDB_MKDIRS "/apps/rippix/cddb/make_directories"
#define CONF_CDDB_CREATID3 "/apps/rippix/cddb/create_id3"
#define CONF_CDDB_CREATPL "/apps/rippix/cddb/create_playlist"
#define CONF_CDDB_AUTOLOOKUP "/apps/rippix/cddb/autolookup"
#define CONF_CDDB_FORMATSTR "/apps/rippix/cddb/format_string"
#define CONF_CDDB_DIRFORMATSTR "/apps/rippix/cddb/dir_format_string"

#include <glib.h>
#include <gconf/gconf.h>

/* Query the gconf database for configurations.
   Possible values for key:
   CONFIG_KEY_USERNAME, CONFIG_KEY_USEREMAIL, CONFIG_KEY_GPGKEY
   CONFIG_KEY_EXPOSEEMAIL

  Returns: The value of the key, cast it specifally.
 */
gpointer
config_read (const gchar *key);

/* Writes values to the GConf database.
   data_value must be a pointer which can be casted correctly according
   to data_type. key is the configuration key in GConf database. */
int
config_write (const GConfValueType data_type, const gchar *key,
	      const gpointer data_value);

/* Opens or closes the connection to the GConf database. This function is
   automatically invoked when calling config_read or config_write.
   op must be either CONFIG_OP_OPEN or CONFIG_OP_CLOSE. */
void
config_open_close (const gint op);

#endif // RW_CONFIG_H
