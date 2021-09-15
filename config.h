#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BAR_LENGTH 256
#define SIGWRITE SIGUSR1

typedef __sighandler_t sighandler_t;
typedef void (*MainLoop)(int signum);
typedef struct {
	char *str;
	MainLoop main_loop;
	int signum;
	sighandler_t handler;
} Block;


char time_str[30];
void time_loop(int signum) {
	while (true) {
		sleep(1);
		raise(signum);
	}
}

void time_handler(int signum) {
	strcpy(time_str, "some test text");
	raise(SIGWRITE);
	return;
}

Block blocks[] = {
	{time_str, time_loop, 40, time_handler},
};
