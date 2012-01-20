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

#ifndef ERR_DIALOG_HANDLER_H
#define ERR_IDALOG_HANDLER_H

#include "common.h"

/* Error codes */
#define FORK_ERR					0
#define PIPE_CREATION_ERR			1
#define NULL_OPEN_ERR				2
#define MALLOC_ERR					3
#define FDOPEN_ERR					4
#define PIPE_READ_ERR				5
#define FILE_DELETE_ERR				6
#define CONFIG_OPEN_ERR				7
#define CONFIG_CREATION_ERR			8
#define PTY_OPEN_ERR				9
#define END_PERROR					10

#define TOO_MANY_ARGS_ERR			END_PERROR + 1
#define JOB_IN_PROGRESS_ERR			END_PERROR + 2
#define NOTHING_TO_DO_ERR			END_PERROR + 3
#define CD_PARANOIA_MISC_ERR		END_PERROR + 4
#define TOO_LONG_FILE_NAME_ERR		END_PERROR + 5

#define EMPTY_ENTRY_ERR				END_PERROR + 6
#define CONFIG_PARSE_ERR			END_PERROR + 7
#define CONFIG_EMPTY_ITEM_ERR		END_PERROR + 8

#define INVALID_FILE_SELECTION_ERR	END_PERROR + 9

/* CDDB Error Codes */
#define CDDB_NO_CONNECT_ERR			END_PERROR + 10
#define CDDB_CONNECT_REFUSED_ERR	END_PERROR + 11
#define CDDB_SERVER_ERR				END_PERROR + 12
#define CDDB_NOT_FOUND_ERR			END_PERROR + 13

/* Newly added ones */
#define RX_PARSING_ERR				END_PERROR + 14
#define PLUGIN_NOT_PRESENT_ERR		END_PERROR + 15
#define CREATING_FILE_ERROR			END_PERROR + 16
#define WAV_PATH_NOT_WRITABLE_ERROR	END_PERROR + 17
#define MP3_PATH_NOT_WRITABLE_ERROR	END_PERROR + 18
#define WAV_PATH_CREATE_ERROR		END_PERROR + 19
#define MP3_PATH_CREATE_ERROR		END_PERROR + 20

/* more distinct cdparanoia error codes */
#define CD_PARANOIA_NO_DISC			END_PERROR + 21
#define CD_PARANOIA_NO_PERMS		END_PERROR + 22

/* Dialog codes */
#define DL_ABORT_CONFIRM            0
#define DL_DELETE_ON_ABORT          1
#define DL_OVERWRITE_CONFIRM        2
#define DL_ENTER_FILE_NAME          3
#define DL_CREATE_CONFIG_CONFIRM    4
#define DL_WAV_PART_LOW             5
#define DL_MP3_PART_LOW             6

/* Status codes */
#define STAT_FINISH_WAV				0
#define STAT_FINISH_MP3				1

/* For dialog handler */
#define DL_OK_PRESSED				100

/* Function Prototypes */
void err_handler( int err_code, const char *extra_msg );

int dialog_handler( int ops, int ok_or_yes, int dialog_code,
                    int with_entry, char *entry_default,
                    char *entered, int answer_length );

void status_handler( int status_code, const char *extra_msg );
#endif
