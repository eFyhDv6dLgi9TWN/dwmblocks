#include <stdio.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <unistd.h>
#include <time.h>
#include "dwmblocks.h"
#include "config.h"

static char time_str[16];

static void *time_loop(int);
static void time_handler(int);

Block blocks[] = {
	/* string,   main loop, signum, signal handler. */
	{  time_str, time_loop, 40,     time_handler },
};
const int block_count = sizeof(blocks) / sizeof(blocks[0]);


static time_t time_buffer;
static struct tm *time_buffer_h;

static noreturn void *time_loop(int signum)
{
	while (true) {
		raise(signum);
		sleep(60 - time_buffer_h->tm_sec);
	}
}

static void time_handler(int signum)
{
	time(&time_buffer);
	time_buffer_h = localtime(&time_buffer);
	sprintf(time_str, "🕒 %02d:%02d", 
			time_buffer_h->tm_hour, time_buffer_h->tm_min);
	raise(SIGWRITE);
	return;
}
