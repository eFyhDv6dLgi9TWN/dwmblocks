#ifndef DWMBLOCKS_H
#define DWMBLOCKS_H 1


#include <signal.h>

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
