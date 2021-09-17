// According to the GNU manual, a header can be included
// more than once, so I'm including them anyway.
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// The total maximum length of the bar.
#define BAR_LENGTH 256
// Raise this when you want write the status.
#define SIGWRITE SIGUSR1

// An example of showing time.
#include <time.h>
static char time_str[16];
static time_t time_buffer;
static struct tm *time_buffer_h;
void time_loop(int signum) {
	while (true) {
		time(&time_buffer);
		time_buffer_h = localtime(&time_buffer);
		sleep(60 - time_buffer_h->tm_sec);
		for (int i = 0; i < 60; i++) {
			raise(signum);
			sleep(60);
		}
	}
}

void time_handler(int signum) {
	time(&time_buffer);
	time_buffer_h = localtime(&time_buffer);
	sprintf(time_str, "🕒 %02d:%02d", 
			time_buffer_h->tm_hour, time_buffer_h->tm_min);
	raise(SIGWRITE);
	return;
}


// Some nice type definitions.
typedef __sighandler_t sighandler_t;
typedef void (*MainLoop)(int signum);

typedef struct {
	// The string to be printed on status bar.
	char *str;

	// The main loop function.
	// Note when this is NULL, the last three attribute will be ignored, but str is still going to be shown.
	MainLoop main_loop;

	// The signal to trigger the block.
	int signum;

	// The signal handler to modify the string and let the program write it.
	sighandler_t handler;
} Block;

// Check out the explanation above.
Block blocks[] = {
	{time_str, time_loop, 40, time_handler},
};
