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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

/* plugin for toolame versin 0.2i */

#define OUTPUT_BUF_LENGTH	1024
#define OFFSET_LENGTH		16
#define PRINTOUT_INTERVAL	0.2

int
toolame_read_stat (int *current)
{
	char temp[OUTPUT_BUF_LENGTH];
	char string[100];
	char *string1;
	int bytes_read;
	/* Grab new ouput from 'toolame'  */
	bytes_read = read (0, (void *) temp, sizeof (temp));
        if (bytes_read > OFFSET_LENGTH) {
                sscanf (&temp[1], "%s]", string);
                *current = atoi(string);
                return (0);
        }
	return (-1);
}

int
main (int argc, char **argv)
{
	int begin, length;
	int current;

	sscanf (argv[1], "%d", &begin);
	sscanf (argv[2], "%d", &length);

	while (1) { 
		if (toolame_read_stat (&current) == 0) {
			printf ("[P %f]\n", ((double) current / (double) length) );
		}
		usleep (PRINTOUT_INTERVAL * 1000000);
	}
}
