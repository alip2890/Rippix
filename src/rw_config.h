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
