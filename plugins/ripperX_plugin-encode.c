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
#include <sys/ioctl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define AENCODER_OUTPUT_BUF_LENGTH  1024
#define AENCODER_OUTPUT_LINE_LENGTH 19
#define AENCODER_STATUS_LENGTH	5

#define CD_SECTORS_PER_FRAME        (float)1.959

#define PRINTOUT_INTERVAL           0.5

int aEncoder_read_stat (unsigned *current);
int find_aEncoder_output_read_offset (char *buf, int begin, int end);
void print_msg (int length, int current);

int
aEncoder_read_stat (unsigned *current)
{
	static char buf[AENCODER_OUTPUT_BUF_LENGTH];
	static char status[AENCODER_STATUS_LENGTH];
	int temp_offset, read_offset;
	ssize_t bytes_read;
	int bytes_avail, count;
	static int prev_bytes_avail;

	ioctl (0, FIONREAD, &bytes_avail);
	if (bytes_avail < 4 * AENCODER_OUTPUT_LINE_LENGTH) {
		if (bytes_avail == prev_bytes_avail)
			/* nothing available, let's wait */
			return -1;
		else {
			/* Record available bytes, and let's just wait */
			prev_bytes_avail = bytes_avail;
			return -1;
		}
	}
	prev_bytes_avail = -1;

	count = 0;
	do {
		bytes_read = read (0, (void *) buf, sizeof (buf));
		temp_offset = bytes_read - 4 * AENCODER_OUTPUT_LINE_LENGTH - 1;

		read_offset = find_aEncoder_output_read_offset (buf,
		              temp_offset,
		              sizeof (buf) - 1);
		if (read_offset < 0 ||
			        read_offset > sizeof (buf) - AENCODER_OUTPUT_LINE_LENGTH) {
			if (count == 0)
				return -1;
			else
				break;
		}

		strncpy (status, buf + read_offset, sizeof (status));
		status[sizeof (status) - 1] = '\0';
		sscanf (status, "%u", current);
		/* Convert it to cd sector unit */
		*current *= CD_SECTORS_PER_FRAME;
		count++;
	} while (bytes_read == sizeof (buf));

	if (*current != 0)
		return 0;

	return -1;
}

int
find_aEncoder_output_read_offset (char *buf, int begin, int end)
{
	int i;

	i = begin;

	do {
		while (buf[i] != '[' && i <= end - 5)
			i++;

		if (buf[i + 1] == ' ' && isdigit (buf[i + 5]))
			return i + 2;
		else
			i++;
	} while (i <= end - 5);
	return -1;
}

// print out [P 0.xxxx]\n
void
print_msg (int length, int current)
{
	printf ("[P ");
	printf ("%f]\n", (double) current / (double) length);
}

int
main (int argc, char **argv)
{
	int begin, length;
	int current;

	if (argc != 3) {
		fprintf (stderr, "This is ripperX plugin for the ISO encoder version 2. Syntax is\n"
		         "ripperX_plugin-encoder beginning_sector length_in_sector\n");
		exit (1);
	}

	sscanf (argv[1], "%d", &begin);
	sscanf (argv[2], "%d", &length);

	print_msg (length, 0);
	while (1) {
		if (aEncoder_read_stat (&current) == 0)
			print_msg (length, current);
		usleep (PRINTOUT_INTERVAL * 1000000);
	}
}
