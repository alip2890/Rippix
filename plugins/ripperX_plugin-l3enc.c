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

#define OUTPUT_BUF_LENGTH 4096
#define OFFSET_LENGTH 22

#define PRINTOUT_INTERVAL       0.5

int
l3enc_read_stat (unsigned int *percent)
{
	char temp[OUTPUT_BUF_LENGTH];
	char *string1;
	int bytes_read;
	/* Grab new ouput from 'l3enc'  */
	bytes_read = read (0, (void *) temp, sizeof (temp));
	/* This is ugly, but then again so is the output of l3enc */
	if (bytes_read > OFFSET_LENGTH) {
		string1 = temp + OFFSET_LENGTH;
		string1[3] = 0;
		*percent = atoi (string1);
		return (0);
	}
	return (-1);
}

int
main (int argc, char **argv)
{
	unsigned int percent = 0;

	while (1) {
		if (l3enc_read_stat (&percent) == 0)
			/* print message in form [P 0.xxxxx]\n */
			printf ("[P %f]\n", ((float) (percent)) / 100.0);
		usleep (PRINTOUT_INTERVAL * 1000000);
	}
}
