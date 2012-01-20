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

#ifndef RIPPER_ENCODER_MANIPULATION_H
#define RIPPER_ENCODER_MANIPULATION_H

#include "common.h"

#define BUF_LENGTH_FOR_F_SCAN_CD    150

#define MAX_PLUGIN_OUTPUT_LENGTH    1024

// return values of read_and_process... & parse_plugin_output
#define PLUGIN_MSG_PARSE_ERR        -2
#define PLUGIN_NO_MSG_AVAILABLE     -1
#define PLUGIN_PROGRESS_MSG         0
#define PLUGIN_WARN_MSG             1
#define PLUGIN_ERR_MSG              2

int scan_cd( _main_data *main_data );

int start_ripping_encoding( int type, int begin, int length,
                            int track,
                            char *src_file_name, char *dest_file_name,
                            int *program_pid, int *plugin_pid,
                            int *read_fd );
// type is either WAV(rip) or MP3(encode)

int read_and_process_plugin_output( int read_fd, double *progress, char *msg );

#endif
