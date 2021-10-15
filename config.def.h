#ifndef CONFIG_H
#define CONFIG_H 1


#include <time.h>
#include "dwmblocks.h"

// The filename length of /run/user/$uid/dwmblocks.pid
#define PATH_LEN 256
// The total maximum length of the bar.
#define BAR_LENGTH 256

// An example of showing time.
static char time_str[16];
static time_t time_buffer;
static struct tm *time_buffer_h;
static inline void *time_loop(int signum) {
	while (true) {
		raise(signum);
		sleep(60 - time_buffer_h->tm_sec);
	}
}

static inline void time_handler(int signum) {
	time(&time_buffer);
	time_buffer_h = localtime(&time_buffer);
	sprintf(time_str, "🕒 %02d:%02d", 
			time_buffer_h->tm_hour, time_buffer_h->tm_min);
	raise(SIGWRITE);
	return;
}

// string, main loop, signal number, signal handler.
Block blocks[] = {
	{time_str, time_loop, 40, time_handler},
};


#endif
