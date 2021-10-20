/*
 * Edited: Yushun Cheng, from 2021-09-15 +0800.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include "dwmblocks.h"
#include "config.h"


static char status_str[2][BAR_LENGTH];
// X related.
static Display *display = NULL;
static int screen;
static Window root;
// Signals.
static const int sigterm[] = {
	SIGHUP, SIGINT, SIGTERM
};

static void (*write_status)();


static int get_status()
{
	strcpy(status_str[1], status_str[0]);
	status_str[0][0] = '\0';
	for(int i = 0; i < block_count; i++) {
		strcat(status_str[0], " ");
		strcat(status_str[0], blocks[i].str);
		strcat(status_str[0], " ");
        if (i != block_count - 1)
		strcat(status_str[0], "|");
	}
	return strcmp(status_str[0], status_str[1]);
}

// Signal Handlers
static void termhandler(int signum) {
	if (display) XCloseDisplay(display);
	exit(128 + signum);
	return;
}

static void setroot(int signum)
{
	if (get_status() == 0)  // Only write out if text has changed.
		return;

	XStoreName(display, root, status_str[0]);
	XFlush(display);

	return;
}

static void pstdout(int signum)
{
	if (get_status() == 0)
		return;

	puts(status_str[0]);
	fflush(stdout);
	return;
}

// Thread Function
static void *thread_routine(void *arg)
{
	Block *block = (Block *) arg;

	return (*(block->main_loop))(block->signum);
}

// Other Static Functions
static void exit_fail() {
	if (display) XCloseDisplay(display);
	exit(EXIT_FAILURE);
}

static void setup_x() {
	display = XOpenDisplay(NULL);
	if (!display) {
		pferror("XOpenDisplay", "Error.");
		goto exit_fail;
	}

	screen = DefaultScreen(display);
	root = RootWindow(display, screen);

	return;

exit_fail:
	exit_fail();
}

static void write_pid() 
{
	int err;
	char *runtime_dir;
	char pid_filename[PATH_LEN];
	FILE *pid_file;

	// Prepare the pid file name.
	runtime_dir = getenv("XDG_RUNTIME_DIR");
	if (!runtime_dir) {
		err = sprintf(pid_filename, "/run/user/%d/dwmblocks.pid",
				getuid());
		if (err < 0) {
			perror("sprintf");
			goto exit_fail;
		}
	} else {
		strcpy(pid_filename, runtime_dir);
		strcat(pid_filename, "/dwmblocks.pid");
	}

	// Write the pid.
	pid_file = fopen(pid_filename, "w");
	if (pid_file == NULL) {
		perror("fopen");
		goto exit_fail;
	}
	err = fprintf(pid_file, "%d", getpid());
	if (err < 0) {
		perror("fprintf");
		goto exit_fail;
	}
	err = fclose(pid_file);
	if (err) {
		perror("fclose");
		goto exit_fail;
	}

	return;

exit_fail:
	exit_fail();
}

static void setup_signal(int signum, sighandler_t handler) {
	if (signal(signum, handler) == SIG_ERR) {
		perror("signal");
		exit_fail();
	}
}

static void setup() 
{
	int err;
	pthread_t thread_id;
	Block *pBlock = blocks;

	for (int i = 0; i < sizeof(sigterm) / sizeof(sigterm[0]); i++)
		setup_signal(sigterm[i], termhandler);
	setup_signal(SIGWRITE, write_status);
	for (int i = 0; i < block_count; i++) {
		setup_signal(pBlock->signum, pBlock->handler);
		if (!pBlock->main_loop) continue;  // Skip NULL loops.
		err = pthread_create(&thread_id, NULL, thread_routine, 
				(void *) pBlock);
		if (err) {
			pferror("pthread_create", "Error.");
			goto exit_fail;
		}
		++pBlock;
	}

	return;

exit_fail:
	exit_fail();
}

// Main
int main (int argc, char *argv[]) 
{
	setup_x();

	write_status = setroot;
	for (int i = 0; i < argc; i++)
		if (!strcmp(argv[i], "-p")) {
			write_status = pstdout;
			XCloseDisplay(display);
			display = NULL;
		}

	write_pid();
	setup();
	while (true) pause();  // Receiving signals.
}
