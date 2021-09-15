#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <X11/Xlib.h>

#include "config.h"

#define SIGSTOPTHRD SIGUSR2


static pthread_t threads[COUNT(blocks)];
static pthread_t control_thread;
static int thread_count = 0;
static char status_str[2][BAR_LENGTH];


int get_status()
{
	strcpy(status_str[1], status_str[0]);
	status_str[0][0] = '\0';
    for(int i = 0; i < COUNT(blocks); i++) {
		strcat(status_str[0], " ");
		strcat(status_str[0], blocks[i].str);
        if (i != COUNT(blocks) - 1)
            strcat(status_str[0], " |");
    }
	return strcmp(status_str[0], status_str[1]);
}

// Signal Handlers
void stop_thread(int signum) {
	pthread_exit(NULL);
}

void setroot(int signum)
{
	static Display *display;
	int screen;
	Window root;

	if (get_status() == 0)
		return;

	Display *d = XOpenDisplay(NULL);
	if (d)
		display = d;

	screen = DefaultScreen(display);
	root = RootWindow(display, screen);
	XStoreName(display, root, status_str[0]);
	XCloseDisplay(display);

	return;
}

void pstdout(int signum)
{
	fflush(stdout);
	if (get_status() == 0)//Only write out if text has changed.
		return;

	printf("%s\n",status_str[0]);
	fflush(stdout);
	return;
}

// Thread Function
void *block_thrd_func(void *arg) {
	Block *block = (Block *) arg;

	signal(block->signum, block->handler);

	(*(block->main_loop))(block->signum);
	return NULL;
}

void (*write_status)(int signum) = setroot;

int main (int argc, char *argv[]) {
	printf("SIGRTMIN: %d", SIGRTMIN);
	for (int i = 0; i < argc; i++)
		if (!strcmp(argv[i], "-p"))
			write_status = pstdout;

	for (int i = 0; i < COUNT(blocks); i++) {
		if (pthread_create(threads+i, NULL, 
					block_thrd_func, (void *) (blocks+i))
				!= 0) {
			printf("Fail to create the thread No. %d.\n", i);
			// This signal can be sent before other child signals 
			// block them; need to fix.
			raise(SIGABRT);
		}
		++thread_count;
	}

	signal(SIGWRITE, write_status);
	pause();
}
