#ifndef READFILE_H
#define READFILE_H 1


#include <stdio.h>
#include <stdbool.h>

#ifndef STRLEN
#define STRLEN 64
#endif

static inline void readfile(const char *path, char *buf) {
	size_t size;
	FILE *stream;

	stream = fopen(path, "r");
	if (!stream) {
		perror("fopen");
		return;
	}

	while (true) {
		size = fread(buf, 1, STRLEN, stream);
		if (size < STRLEN) break;
		buf += STRLEN;
	}

	return;
}

#endif
