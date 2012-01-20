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

#ifndef PLAYERS_MANIPULATION_H
#define PLAYERS_MANIPULATION_H

#include "common.h"

#define PLAY       0
#define STOP       1

char **players_create_argv( int ops, int cd_wav_mp3, char *playit );

int play_cd_wav_mp3( int ops, int cd_wav_mp3, char *playit );
/* The player. This function return TRUE on success. FALSE on failure.
 * This function uses players_create_argv function.
 * ops selects to play or to stop. playit
 * is a string which contains the number of track if ops is CD(that is "3". 
 * Otherwise, it should contain the name of file to play */

#endif
