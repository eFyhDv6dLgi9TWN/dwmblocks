#ifndef AUDIO_H
#define AUDIO_H 1


#ifndef STRLEN
#define STRLEN 64
#endif

#include <stdio.h>
#include <signal.h>
#include "dwmblocks.h"

static char audio_str[STRLEN];

static inline void *audio_loop(int signum) {
	raise(signum);
	return NULL;
}

static inline void audio_handler(int signum) {
	static int level;
	static FILE *stream;

	stream = popen("amixer -c 1 cget numid=12 |"
			"grep \": values\"", "r");
	fscanf(stream, "  : values=%d", &level);
	pclose(stream);

	sprintf(audio_str, "🔈 -%d dB", 74 - level);
	raise(SIGWRITE);
	return;
}


#endif
