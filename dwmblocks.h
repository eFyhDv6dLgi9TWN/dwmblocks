#ifndef DWMBLOCKS_H
#define DWMBLOCKS_H 1


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <X11/Xlib.h>

#define SIGWRITE SIGUSR1
#define pferror(f, msg) fprintf(stderr, "%s: %s\n", f, msg)

typedef __sighandler_t sighandler_t;
typedef struct {
	char *str;
	void *(*main_loop)(int signum);
	int signum;
	sighandler_t handler;
} Block;


#endif
