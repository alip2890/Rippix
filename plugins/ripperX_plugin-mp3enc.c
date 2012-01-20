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

#define OUTPUT_BUF_LENGTH  1024


#define PRINTOUT_INTERVAL       0.5

int
read_stat (unsigned int *current, unsigned int *length)
{
	char temp[OUTPUT_BUF_LENGTH];
	int bytes_read;
	/* Grab new ouput from 'gogo'  */
	bytes_read = read (0, (void *) temp, sizeof (temp));
	if (bytes_read) {
		sscanf (temp, "%u / %u", current, length);
		return (0);
	}
	return (-1);
}

int
main (int argc, char **argv)
{
	unsigned int length = 1;
	unsigned int current = 0;

	while (1) {
		if (read_stat (&current, &length) == 0)
			/* print message in form [P 0.xxxxx]\n */
			printf ("[P %f]\n", (double) current / (double) length);
		usleep (PRINTOUT_INTERVAL * 1000000);
	}
}
