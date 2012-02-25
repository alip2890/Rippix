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
#include "config.h"
#endif

#include <locale.h>
#include "gettext.h"
#include "interface_common.h"
#include "main_window_handler.h"
#include "err_dialog_handler.h"
#include "job_control.h"
#include "rw_config.h"

#include "main.h"

void ripperX_init (_main_data * main_data);

/* Global variable */
_config config;
int where_now;


void
ripperX_init (_main_data * main_data)
{
  memset (main_data, 0, sizeof (_main_data));
  read_config ();
  main_window_handler (WIDGET_CREATE, 0, main_data);
  return;
}


void
ripperX_exit (GtkWidget * widget, gpointer callback_data)
{
  if (where_now == STATUS_FRAME)
    {
      job_controller (JC_PAUSE, NULL);

      /* Confirm */
      if (dialog_handler (WIDGET_CREATE, FALSE, DL_ABORT_CONFIRM,
			  FALSE, NULL, NULL, 0) == FALSE)
	{
	  job_controller (JC_CONT, NULL);
	  return;
	}

      /* Terminate current job */
      if (dialog_handler (WIDGET_CREATE, FALSE, DL_DELETE_ON_ABORT,
			  FALSE, NULL, NULL, 0) == TRUE)
	job_controller (JC_ABORT_ALL_DELETE, NULL);
      else
	job_controller (JC_ABORT_ALL, NULL);
    }

  main_window_handler (WIDGET_DESTROY, NULL, NULL);
  gtk_main_quit ();
}

int
main (int argc, char *argv[])
{
  _main_data main_data;
  gtk_init (&argc, &argv);

  setlocale (LC_ALL, "");
  setlocale (LC_NUMERIC, "POSIX");
#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain (PACKAGE);
#endif /*ENABLE_NLS */

  ripperX_init (&main_data);
  gtk_main ();
  return 0;
}
