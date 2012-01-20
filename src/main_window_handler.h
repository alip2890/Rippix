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

#ifndef MAIN_WINDOW_HANDLER_H
#define MAIN_WINDOW_HANDLER_H

#include "common.h"
#include <gtk/gtk.h>

#define MW_MODE_SELECT              100
#define MW_MODE_STATUS              101
#define MW_MODE_CONFIG              102
#define MW_CLEAR_STATUSBAR          103
#define MW_UPDATE_STATUSBAR         104
#define MW_REQUEST_MF               105
#define NO_CD_INDRIVE               106
#define CD_INDRIVE                  107

/* Creates main window. Returns the pointer to main frame which will be
 * used by select frame and update frames. ops can be one of WIDGET_CREATE,
 * MW_MODE_SELECT, MW_MODE_STATUS, MW_MODE_CONFIG, MW_UPDATE_STATUSBAR *
 *
 * main_data is used to install button callbacks */
GtkWidget *main_window_handler( int ops, char *status_bar_msg,
                                _main_data *main_data );

void mw_scan_button_clicked( GtkWidget *widget, gpointer callback_data );
void mw_cddb_button_clicked( GtkWidget *widget, gpointer callback_data );

#endif
