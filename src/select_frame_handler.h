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

#ifndef SELECT_FRAME_HANDLER_H
#define SELECT_FRAME_HANDLER_H

#include "common.h"
#include <gtk/gtk.h>

#define SF_SELECT_BUTTON_ACT		101
#define SF_ALBUM_ENTRY_CHG			102
#define SF_FILENAME_ENTRY_CHG		103
#define SF_ARTIST_ENTRY_CHG			104
#define SF_SELECT_BUTTON_ACT_ALL	105
#define SF_SYNC_MAIN_DATA			106
#define SF_SYNC_SELECT_FRAME		107
#define SF_SELECT_ALL				108

#define SF_YEAR_ENTRY_CHG			110
#define SF_GENRE_ENTRY_CHG			111

/* Create or destroy select frame. ops can be one of WIDGET_CREATE,
 * WIDGET_DESTROY, SF_WAV_BUTTON_ACT, SF_MP3_BUTTON_ACT, 
 * , SF_WAV_ENTRY_CHG, SF_MP3_ENTRY_CHG, and SF_SYNC_MAIN_DATA.
 * When called with WIDGET_CREATE, it will save the value of *main_data
 * and use it for further operation until the widget is created again.
 * So, it's important not to change *main_data while this function is
 * operating on it */
/* Call back functions will convert gpointer callback_data to an integer */
void select_frame_handler( int ops, int track, _main_data *main_data );

#endif
