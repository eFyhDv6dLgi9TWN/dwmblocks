#ifndef BATTERY_H
#define BATTERY_H 1


#ifndef STRLEN
#define STRLEN 64
#endif

#ifndef FLASH_RATE
#define FLASH_RATE 500
#endif

#define BATPATH "/sys/class/power_supply/BAT0"
#define BATWARN 5

#include <stdlib.h>
#include "dwmblocks.h"
#include "readfile.h"

static char bat_str[STRLEN];
static long long bat_level;
static char bat_status[STRLEN];

static inline void *bat_loop(int signum) {
	while (true) {
		raise(signum);
		sleep(1);
		while (bat_level < BATWARN &&
				strcmp(bat_status, "Charging")) {
			raise(signum);
			usleep(FLASH_RATE);
			*bat_str = '\0';
			usleep(FLASH_RATE);
		}
		sleep(60);
	}
}

static inline void bat_handler(int signum) {
	static char charge_now[STRLEN];
	static char charge_full_design[STRLEN];
	static const char *status_emoji;
	static char color_char;

	readfile(BATPATH "/charge_now", charge_now);
	readfile(BATPATH "/charge_full_design", charge_full_design);
	readfile(BATPATH "/status", bat_status);

	bat_level = atoll(charge_now) * 100 / atoll(charge_full_design);
	color_char = '\x03' + (bat_level - 1) / 10;
	if (!strcmp(bat_status, "Charging") || !strcmp(bat_status, "Full"))
		status_emoji = "🔌";
	else
		status_emoji = "🔋";

	sprintf(bat_str, "%c%s %lli%%\x01", color_char, status_emoji,
			bat_level);
	raise(SIGWRITE);
	return;
}


#endif
