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

// strndup is a GNU extension:
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define OGG_OUTPUT_BUF_LENGTH 	2048
#define PRINTOUT_INTERVAL	0.5

void 
strip_shit(char* input, int len) {
	int i;
	
	for (i = 0; i < len; i++) {
		if (input[i] < ' ' || input[i] > 'z') {
			input[i] = ' ';
		}
	}
}

int 
extract_perc(char* input, float *result) {
    char* bpos;
    char* epos;
    char* subs;
    size_t ipos;

    bpos = strchr(input, '\x5b');
    epos = strchr(input, '\x25');
    if (bpos != NULL && epos != NULL && epos > bpos) {
        ipos = (size_t)(epos - bpos);
        subs = strndup(bpos + 1, ipos - 1);
        sscanf(subs, "%f", result);
        return 0;
    } else {
        return -1;
    }
}

int
ogg_read_stat (float *perc)
{
	char temp[OGG_OUTPUT_BUF_LENGTH];
	int bytes_read;
	char input[OGG_OUTPUT_BUF_LENGTH];

	/* Grab new ouput from 'oggenc'  */
	bytes_read = read (0, (void *) temp, sizeof (temp));
	if (bytes_read) {
		strip_shit(temp, bytes_read);
		return extract_perc(temp, perc);
	}
	return (-1);
}

int
main (int argc, char **argv)
{
	float perc = 0;
	
	while (1) {
		if (ogg_read_stat (&perc) == 0)
			/* print message in form [P 0.xxxxx]\n */
			printf ("[P %f]\n", perc / 100 );
		usleep (PRINTOUT_INTERVAL * 1000000);
	}
}
