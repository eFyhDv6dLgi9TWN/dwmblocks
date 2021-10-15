#ifndef TIMEDATE_H
#define TIMEDATE_H 1


#include <time.h>
#include "dwmblocks.h"

#ifndef STRLEN
#define STRLEN 64
#endif

static char time_str[STRLEN];
static char date_str[STRLEN];
static time_t time_int;
static struct tm *time_struct;
static const char wdays[7][4] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

static inline void *time_loop(int signum) {
	while (true) {
		raise(signum);
		sleep(1);
		sleep(59 - time_struct->tm_sec);
	}
}

static inline void time_handler(int signum) {
	time(&time_int);
	time_struct = localtime(&time_int);
	sprintf(time_str, "🕒 %02d:%02d", 
			time_struct->tm_hour,
			time_struct->tm_min);
	raise(signum + 1);
	return;
}

static inline void date_handler(int signum) {
	sprintf(date_str, "📅 %04d-%02d-%02d %s",
			time_struct->tm_year+1900,
			time_struct->tm_mon+1,
			time_struct->tm_mday,
			wdays[time_struct->tm_wday]);
	raise(SIGWRITE);
	return;
}


#endif
