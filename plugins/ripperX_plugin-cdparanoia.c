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

#define CDPARANOIA_OUTPUT_BUF_LENGTH             1024
#define CDPARANOIA_OUTPUT_LINE_LENGTH            74
#define CDPARANOIA_STAT_LENGTH      42
#define CDPARANOIA_GRAPH_LENGTH     30

#define PRINTOUT_INTERVAL           0.5

extern int errno;
int cdparanoia_read_stat (unsigned *current, char **graph_string);
int find_cdparanoia_output_read_offset (char *buf, int begin, int end);
void print_msg (int begin, int length, int current, char *graph);

int
cdparanoia_read_stat (unsigned *current, char **graph_string)
{
	extern int errno;
	static char buf[CDPARANOIA_OUTPUT_BUF_LENGTH];
	static char status[CDPARANOIA_STAT_LENGTH + 1];
	static char graph[CDPARANOIA_GRAPH_LENGTH + 1];
	ssize_t bytes_read;
	int bytes_avail;
	static int prev_bytes_avail = -1;
	int read_offset, temp_offset, count;

	ioctl (0, FIONREAD, &bytes_avail);
	if (bytes_avail < 4 * CDPARANOIA_OUTPUT_LINE_LENGTH) {
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
		temp_offset = bytes_read - 4 * CDPARANOIA_OUTPUT_LINE_LENGTH - 1;
		read_offset = find_cdparanoia_output_read_offset (buf,
		              temp_offset,
		              sizeof (buf) - 1);
		if (read_offset < 0
			        || read_offset > sizeof (buf) - CDPARANOIA_OUTPUT_LINE_LENGTH) {
			if (count == 0)
				return -1;
			else
				break;
		}

		strncpy (status, buf + read_offset, sizeof (status));
		status[sizeof (status) - 1] = '\0';
		count++;
	} while (bytes_read == sizeof (buf));

	strncpy (graph, status, sizeof (graph));
	graph[sizeof (graph) - 1] = '\0';
	*graph_string = graph;

	temp_offset = CDPARANOIA_GRAPH_LENGTH + 1;

	sscanf (status + temp_offset + 1, "%u", current);

	return 0;
}

int
find_cdparanoia_output_read_offset (char *buf, int begin, int end)
{
	int i;

	i = begin;

	do {
		while (buf[i] != '=' && i <= end - 16)
			i++;

		if (buf[i + 3] == 'P'
			        && isdigit (buf[i + 16 + CDPARANOIA_GRAPH_LENGTH + 3]))
			return i + 16;
		else
			i++;
	} while (i <= end - 16);
	return -1;
}

// print out [P 0.xxxx "---->          "]\n
void
print_msg (int begin, int length, int current, char *graph)
{
	printf ("[P ");
	printf ("%f ", (double) (current - begin) / (double) length);
	printf ("\"%s\"]\n", graph);
}

int
main (int argc, char **argv)
{
	int begin, length;
	int current;
	char *graph_string;

	if (argc != 3) {
		fprintf (stderr, "This is ripperX plugin for cdparanoia. Syntax is\n"
		         "ripperX_plugin-cdparanoia beginning_sector length_in_sector\n");
		exit (1);
	}

	sscanf (argv[1], "%d", &begin);
	sscanf (argv[2], "%d", &length);

	while (1) {
		if (cdparanoia_read_stat (&current, &graph_string) == 0)
			print_msg (begin, length, current, graph_string);
		usleep (PRINTOUT_INTERVAL * 1000000);
	}
}
