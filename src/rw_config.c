#define _GNU_SOURCE
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

#include <config.h>
#include <locale.h>
#include <glib.h>
#include <glib-object.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-value.h>
#include <error.h>

#include "gettext.h"
#include "rw_config.h"

static GConfClient *conf_client;

gpointer
config_read (const char *key)
{
  /* Create the connection to GConf database if it doesn't exist yet. */
  config_open_close (CONFIG_OP_OPEN);

  if (key)
    {
      GError *read_error = NULL;
      GConfValue *value = gconf_client_get (conf_client, key, &read_error);

      if (read_error)
	{
	  error (0, 0, _("Could not read GConf key: %s"), read_error->message);
	  g_clear_error (&read_error);
	  return NULL;
	}
      else
	{
	  if (value)
	    {
	      switch (value->type)
		{
		case GCONF_VALUE_STRING:
		  return (gpointer) gconf_value_get_string (value);
		  break;
		case GCONF_VALUE_BOOL:
		  return (gpointer) gconf_value_get_bool (value);
		  break;
		case GCONF_VALUE_INT:
		  return (gpointer) gconf_value_get_int (value);
		  break;
		  /* gdouble cannot be casted to gpointer somehow...

		     case GCONF_VALUE_FLOAT:
		     data = (gpointer) gconf_value_get_float (value);
		     break;*/
		case GCONF_VALUE_FLOAT:
		case GCONF_VALUE_SCHEMA:
		case GCONF_VALUE_LIST:
		case GCONF_VALUE_PAIR: 
		case GCONF_VALUE_INVALID:
		  error (0, 0, _("Error reading GConf database: Key type is not supported"));
		  break;
		}
	    }
	  else
	    {
	      error (0, 0, _("Key could not be read: key unset or no default exists"));

	    }
	}
    }
    return NULL;  
}

int
config_write (const GConfValueType data_type, const char *key,
	      const gpointer data_value)
{
  GConfValue *temp_value;

  config_open_close (CONFIG_OP_OPEN);

  temp_value = gconf_value_new (data_type);
  switch (data_type)
    {
    case GCONF_VALUE_STRING:
	gconf_value_set_string (temp_value, (char *) data_value);
	break;
    case GCONF_VALUE_INT:
	gconf_value_set_int (temp_value, (gint) data_value);
	break;
    case GCONF_VALUE_BOOL:
	gconf_value_set_bool (temp_value, (gboolean) data_value);
	break;
    case GCONF_VALUE_FLOAT:
    case GCONF_VALUE_SCHEMA:
    case GCONF_VALUE_LIST:
    case GCONF_VALUE_PAIR:
    case GCONF_VALUE_INVALID:
      {
	error (0, 0, _("Error writing to GConf database: Key type is not supported"));
	return FALSE;
      }
    }

  gconf_client_set (conf_client, key, temp_value, NULL);

  return TRUE;
}

void
config_open_close (const gint op)
{
  static GMutex mutex;

  switch (op)
    {
    case CONFIG_OP_OPEN:
      g_mutex_lock (&mutex);
      if (!conf_client)
	{
	  conf_client = gconf_client_get_default ();
	}
      g_mutex_unlock (&mutex);
      break;
    case CONFIG_OP_CLOSE:
      g_mutex_lock (&mutex);
      if (conf_client)
	{
	  g_object_unref (conf_client);
	}
      g_mutex_unlock (&mutex);
      break;
    }
}
