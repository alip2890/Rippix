/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __VERSION_H__
#define __VERSION_H__

#include <glib.h>

#define MAKE_NUMERIC_VERSION(a, b, c, d) ((((guint32)a) << 24) | (((guint32)b) << 16) | \
					  (((guint32)c) <<  8) |  ((guint32)d)      )

#define PACKAGE			"rippix"
#define VERSION			"3.4"
#define PROG_VERSION		"ripperX "VERSION
#define VERSION_NUMERIC		MAKE_NUMERIC_VERSION(3, 4, \
						     @MICRO_VERSION@, @EXTRA_VERSION@)

#endif /* __VERSION_H__ */
