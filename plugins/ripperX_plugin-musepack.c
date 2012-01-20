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
#include <string.h>

/* plugin for musepack encoder mppenc version 1.16 */

#define MUSE_OUTPUT_BUF_LENGTH  2048
#define PRINTOUT_INTERVAL       1
/* mppenc updates screen at 1 cpu usage second, so use longer
PRINTOUT_INTERVAL to help prevent reading partial lines */


int
mppenc_read_stat (float *current)
{
	char temp[MUSE_OUTPUT_BUF_LENGTH];
	char string[MUSE_OUTPUT_BUF_LENGTH];
	int bytes_read;
	int bytes_left;
	char *kbps = "kbps";
	char *string_ptr;
	char *temp_ptr;

	temp_ptr = temp;
	/* Grab new output from 'mppenc'  */
	bytes_read = read (0, (void *) temp, sizeof (temp));
	if (bytes_read) {
		temp[bytes_read]='\0';
		string_ptr = strstr (temp_ptr, kbps);
		bytes_left = string_ptr - temp_ptr;
		if ( (bytes_left < 20) && (bytes_left > 8) ) { /*discard the long initial header or fragments*/
			if ( temp[0] == '\015' ) {
				temp_ptr = temp;
				temp_ptr++;
				strcpy( string , temp_ptr );
			}
			else {
				strcpy( string , temp);
			}
			sscanf (string, "%f", current);   /*pickup the first number in the string = % done */
			if (!(*current > 100 || *current <= 0 ))  /*just in case bad value */
			return (0);
			
		}
	}
	return (-1);
}

int
main (int argc, char **argv)
{
	float current = 0.0;

	printf ("[P 0.00]\n");
	while (1) {
		if (mppenc_read_stat (&current) == 0)
			/* print message in form [P 0.xxxxx]\n */
			printf ("[P %f]\n", current / 100.0);
		usleep (PRINTOUT_INTERVAL * 1000000);
	}
}
