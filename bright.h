#ifndef BRIGHT_H
#define BRIGHT_H 1


#ifndef STRLEN
#define STRLEN 64
#endif

#define BRIGHT_PATH "/sys/class/backlight/amdgpu_bl0/brightness"
#define BRIGHT_MAX 255

#include <stdlib.h>
#include "readfile.h"
#include "dwmblocks.h"

static char bright_str[STRLEN];

static inline void *bright_loop(int signum) {
	raise(signum);
	return NULL;
}

static inline void bright_handler(int signum) {
	static char brt[STRLEN];

	readfile(BRIGHT_PATH, brt);
	sprintf(bright_str, "💡 %d%%", atoi(brt)*100/BRIGHT_MAX);
	raise(SIGWRITE);
	return;
}


#endif
