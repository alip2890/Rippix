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

#ifndef CDDBP_H
#define CDDBP_H

/* the following functions are active, meaning that they themselves will
	communicate directly to the server through the FILE* which is given */
#if 0
#define DEBUG
#endif

#include "common.h"

int cddbp_signon( FILE* );
int cddbp_handshake( FILE*, const char *clientname, const char *version );
int cddbp_query( FILE*, const char *disk_id, int tracknum,
                 long int offsets[ tracknum ], int duration, int *matches,
                 char ***category_buffer, char ***title_buffer,
                 char ***id_buffer );
int cddbp_read( FILE*, const char *category, const char *disk_id,
                char **result_buffer );

int http_query( const char *server, int port, const char* URL,
                const char *disk_cd, int tracknum, long int offsets[ tracknum ],
                int duration, int *matches,
                char ***category_buffer, char ***title_buffer,
                char ***id_buffer, const char *client, const char *version );

int http_query_proxy( const char *server, int port, const char *proxy_server, int proxy_port, const char *URL,
                      const char *cd_id, int tracknum, long int offset[ tracknum ],
                      int duration, int *matches,
                      char ***category_buffer, char ***title_buffer,
                      char ***id_buffer, const char *client, const char *version );

int http_read( const char *server, int port, const char *URL,
               const char *category, const char *disk_id,
               char **result_buffer, const char *client, const char *version );
int http_read_proxy( const char *server, int port, const char *proxy_server, int proxy_port, const char *URL,
                     const char *category, const char *disk_id,
                     char **result_buffer, const char *client, const char *version );

void cddbp_signoff( FILE* );

#endif

