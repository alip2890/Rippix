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
#include <sys/types.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define TRUE  1
#define FALSE 0

#define PLUGIN_MSG_PARSE_ERR        -1	// also interpreted as 'no more output'
#define PLUGIN_PROGRESS_MSG         0
#define PLUGIN_WARN_MSG             1
#define PLUGIN_ERR_MSG              2

#define CHECK_INTERVAL              0.475

int add_argv (char **dest, char *content);
int open_a_pty (int *pty, int *tty);
char **create_argv_for_execution_using_shell (char *command);
int execute_ripper_encoder_with_plugin (char **program_argv,
                                        char **plugin_argv,
                                        pid_t * program_pid, pid_t * plugin_pid,
                                        int *read_fd);
void err_handler (char *msg);
int read_and_process_plugin_output (int read_fd, double *progress, char *msg);
int parse_plugin_output (char *out, double *progress, char *msg);

int
add_argv (char **dest, char *content)
{
	size_t i;

	i = 0;

	while (content[i++] != '\0');

	if ((*dest = malloc (i)) == NULL) {
		err_handler ("Cannot allocate memory");
		return FALSE;
	}

	strcpy (*dest, content);
	return TRUE;
}

void
err_handler (char *msg)
{
	fprintf (stderr, "Error : %s\n", msg);
}

int
open_a_pty (int *pty, int *tty)
{
	char line[12];
	char *p1, *p2;
	char *cp;
	int i;

	sprintf (line, "/dev/ptyXX");
	p1 = &line[8];
	p2 = &line[9];

	for (cp = "pqrstuvwxyzPQRST"; *cp; cp++) {
		struct stat stb;

		*p1 = *cp;
		*p2 = '0';
		/*
		 * This stat() check is just to keep us from
		 * looping through all 256 combinations if there
		 * aren't that many ptys available.
		 */
		if (stat (line, &stb) < 0)
			break;
		for (i = 0; i < 16; i++) {
			*p2 = "0123456789abcdef"[i];
			*pty = open (line, O_RDWR);
			if (*pty > 0) {
				line[5] = 't';
				/* Now open appropriate tty */
				if ((*tty = open (line, O_RDWR)) < 0) {
					line[5] = 'p';
					close (*pty);
					continue;
				}
				return 0;
			}
		}
	}
	return -1;
}

char **
create_argv_for_execution_using_shell (char *command)
{
	char *shell;
	char **argv;

	shell = "/bin/bash";

	if ((argv = (char **) malloc (sizeof (char *) * 4)) == NULL) {
		err_handler ("malloc failed");
		return NULL;
	}
	argv[0] = NULL;
	argv[1] = NULL;
	argv[2] = NULL;
	argv[3] = NULL;

	if (add_argv (&(argv[0]), shell) == FALSE)
		return NULL;
	if (add_argv (&(argv[1]), "-c") == FALSE)
		return NULL;
	if (add_argv (&(argv[2]), command) == FALSE)
		return NULL;
	return argv;
}

int
execute_ripper_encoder_with_plugin (char **program_argv,
                                    char **plugin_argv,
                                    pid_t * program_pid, pid_t * plugin_pid,
                                    int *read_fd)
{
	int pty_fd0, tty_fd0, pty_fd1, tty_fd1;
	pid_t pid;

	/* Open two pty/tty pairs */
	if (open_a_pty (&pty_fd0, &tty_fd0) < 0) {
		err_handler ("Cannot open pty/tty pair");
		return -1;
	}
	if (open_a_pty (&pty_fd1, &tty_fd1) < 0) {
		close (pty_fd0);
		close (tty_fd0);
		err_handler ("Cannot open pty/tty pair");
		return -1;
	}

	// fork & exec & link plugin
	if ((pid = fork ()) < 0) {
		err_handler ("Cannot fork");
		return -1;
	}
	*plugin_pid = pid;

	if (pid == 0) {
		// We're in the child process
		// save stderr
		int stderr_fd;
		stderr_fd = dup (2);

		dup2 (pty_fd0, 0);
		dup2 (tty_fd1, 1);

		execvp (plugin_argv[0], plugin_argv);

		dup2 (stderr_fd, 2);
		perror ("Failed to exec plugin");
		_exit (127);
	}

	// we're in the parent process
	close (pty_fd0);
	close (tty_fd1);
	*read_fd = pty_fd1;

	// fork the real program
	if ((pid = fork ()) < 0) {
		err_handler ("Cannot fork");
		return -1;
	}
	*program_pid = pid;

	if (pid == 0) {
		// We're in the child process
		// save stderr
		int stderr_fd;
		stderr_fd = dup (2);

		dup2 (tty_fd0, 1);
		dup2 (tty_fd0, 2);

		execvp (program_argv[0], program_argv);

		dup2 (stderr_fd, 2);
		perror ("Failed to exec the specified program");
		_exit (127);
	}
	close (tty_fd0);

	return 0;
}

int
read_and_process_plugin_output (int read_fd, double *progress, char *msg)
{
	int bytes_avail;
	char buf[1024];
	FILE *read_stream;

	ioctl (read_fd, FIONREAD, &bytes_avail);
	if (bytes_avail <= 0) {
		fprintf (stderr, "*** No report available from plugin. Ctrl-C to stop\n");
		// the plugin hasn't printed anything yet. return PLUGIN_MSG_PARSE_ERR
		// which the caller should ignore.
		return PLUGIN_MSG_PARSE_ERR;
	}

	// all the lines are terminated with '\n' and if the plugin started to
	// print something then it'll finish it soon. so using fgets is
	// reasonable

	read_stream = fdopen (read_fd, "r");
	if (fgets (buf, sizeof (buf), read_stream) == NULL)
		return PLUGIN_MSG_PARSE_ERR;
	fprintf (stderr, "*** Output read from plugin : %s", buf);
	return parse_plugin_output (buf, progress, msg);
}

int
parse_plugin_output (char *out, double *progress, char *msg)
{
	int pos, done, s, d;
	char ch;

	*progress = -1;
	msg[0] = '\0';

	pos = 0;
	while (out[pos] != '\0' && out[pos] != '[')
		pos++;

	// parse err point 0 : cannot find beginning '['
	if (out[pos] != '[') {
		fprintf (stderr, "*** parse err at : 0 : cannot find leading '['\n");
		return PLUGIN_MSG_PARSE_ERR;
	}
	pos++;

	// read the type character
	ch = out[pos++];

	if (ch == 'P')
		// if it's a msg reporting progess, read the percentage
		sscanf (out + pos, "%lf", progress);

	while (out[pos] != '\0' && out[pos] != '"' && out[pos] != ']')
		pos++;

	if (out[pos] == '"') {
		// we've got some message
		pos++;

		// copy the message
		done = 0;
		s = pos;
		d = 0;
		while (!done) {
			if (out[s] != '\\' && out[s] != '"' &&
				        out[s] != ']' && out[s] != '\0')
				msg[d++] = out[s++];
			else if (out[s] == '\\') {
				msg[d] = out[s + 1];
				s += 2;
				d++;
			} else {
				msg[d] = '\0';
				done = 1;
			}
		}
	}

	switch (ch) {
	case 'P':
		// parse err point 1 : invalid progress
		if (*progress < 0 || *progress > 1) {
			fprintf (stderr, "*** parse err at : 1 : invalid progress : %f\n",
			         *progress);
			return PLUGIN_MSG_PARSE_ERR;
		}
		return PLUGIN_PROGRESS_MSG;
	case 'W':
		// parse err point 2 : warning report without msg
		if (msg[0] == '\0') {
			fprintf (stderr, "*** parse err at : 2 : warning report w/o msg\n");
			return PLUGIN_MSG_PARSE_ERR;
		}
		return PLUGIN_WARN_MSG;
	case 'E':
		// parse err point 3 : error report without msg
		if (msg[0] == '\0') {
			fprintf (stderr, "*** parse err at : 3 : error report w/o msg\n");
			return PLUGIN_MSG_PARSE_ERR;
		}
		return PLUGIN_ERR_MSG;
	default:
		return PLUGIN_MSG_PARSE_ERR;
	}
}

int
main (int argc, char **argv)
{
	char *pg_com, *pi_com;
	char **pg_argv, **pi_argv;
	pid_t pg_pid, pi_pid;
	int read_fd;
	double progress;
	char msg[1024];
	int rs;

	if (argc != 3) {
		fprintf (stderr, "Syntax: ripperX_plugin_tester"
		         " \"program command with args\" \"plugin command\"\n");
		exit (1);
	}
	pg_com = argv[1];
	pi_com = argv[2];
	fprintf (stdout, "***Executing \"%s\"\n"
	         "with plugin \"%s\"\n", pg_com, pi_com);

	pg_argv = create_argv_for_execution_using_shell (pg_com);
	pi_argv = create_argv_for_execution_using_shell (pi_com);

	pg_pid = 0;
	pi_pid = 0;

	if (execute_ripper_encoder_with_plugin (pg_argv, pi_argv, &pg_pid, &pi_pid,
		                                        &read_fd) < 0)
		exit (1);

	while (1) {
		usleep (CHECK_INTERVAL * 1000000);
		printf ("\n");
		if ((rs = read_and_process_plugin_output (read_fd, &progress, msg))
			        == PLUGIN_MSG_PARSE_ERR) {
			//if (msg != NULL)
			//free(msg);
			fprintf (stderr, "*** PLUGIN_MSG_PARSE_ERR returned\n");
			continue;
		}
		switch (rs) {
		case PLUGIN_PROGRESS_MSG:
			printf ("Report type : progress, Progress : %f,\nMsg : %s\n",
			        progress, msg);
			break;
		case PLUGIN_WARN_MSG:
			printf ("Report type : warning,\nMsg : %s\n", msg);
			break;
		case PLUGIN_ERR_MSG:
			printf ("Report type : error,\nMsg : %s\n", msg);
			break;
		}
	}

	return 0;
}
