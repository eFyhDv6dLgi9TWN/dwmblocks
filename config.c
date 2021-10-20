#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "dwmblocks.h"
#include "config.h"

/* MACROS */
#define STRLEN 64
#define FLASH_RATE 500  // In microseconds.
/* Battery */
#define BATWARN 5
#define BATPATH "/sys/class/power_supply/BAT0"
/* Brightness */
#define BRIGHT_PATH "/sys/class/backlight/amdgpu_bl0/brightness"
#define BRIGHT_MAX 255

/* GLOBAL VARIABLES */
static char time_str[STRLEN];
static char date_str[STRLEN];
static char bat_str[STRLEN];
static char bright_str[STRLEN];
static char audio_str[STRLEN];

/* GLOBAL FUNCTIONS */
/* Main Loops */
static void *time_loop(int);
static void *bat_loop(int);
static void *bright_loop(int);
static void *audio_loop(int);
/* Signal Handlers */
static void date_handler(int);
static void time_handler(int);
static void bat_handler(int);
static void bright_handler(int);
static void audio_handler(int);

Block blocks[] = {
	{ audio_str,  audio_loop,  38, audio_handler  },
	{ bright_str, bright_loop, 39, bright_handler },
	{ bat_str,    bat_loop,    34, bat_handler    },
	{ time_str,   time_loop,   35, time_handler   },
	{ date_str,   NULL,        36, date_handler   },
};
const int block_count = sizeof(blocks) / sizeof(blocks[0]);

/* STATIC FUNCTIONS */
static void readfile(const char *path, char *buf)
{
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
	buf[size] = '\0';

	return;
}

/* TIME & DATE */
static time_t time_int;
static struct tm *time_struct;
static const char wdays[7][4] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

static noreturn void *time_loop(int signum)
{
	while (true) {
		raise(signum);
		sleep(1);
		sleep(59 - time_struct->tm_sec);
	}
}

static void time_handler(int signum)
{
	time(&time_int);
	time_struct = localtime(&time_int);
	sprintf(time_str, "🕒 %02d:%02d", 
			time_struct->tm_hour,
			time_struct->tm_min);
	raise(signum + 1);
	return;
}

static void date_handler(int signum)
{
	sprintf(date_str, "📅 %04d-%02d-%02d %s",
			time_struct->tm_year+1900,
			time_struct->tm_mon+1,
			time_struct->tm_mday,
			wdays[time_struct->tm_wday]);
	raise(SIGWRITE);
	return;
}

/* BATTERY */
static long long bat_level;
static char bat_status[STRLEN];

static noreturn void *bat_loop(int signum)
{
	while (true) {
		raise(signum);
		sleep(1);
		while (bat_level < BATWARN &&
				strcmp(bat_status, "Charging\n")) {
			raise(signum);
			usleep(FLASH_RATE);
			*bat_str = '\0';
			usleep(FLASH_RATE);
		}
		sleep(60);
	}
}

static void bat_handler(int signum)
{
	static char charge_now[STRLEN];
	static char charge_full_design[STRLEN];
	static const char *status_emoji;
	static char color_char;

	readfile(BATPATH "/charge_now", charge_now);
	readfile(BATPATH "/charge_full_design", charge_full_design);
	readfile(BATPATH "/status", bat_status);

	bat_level = atoll(charge_now) * 100 / atoll(charge_full_design);
	color_char = '\x03' + (bat_level - 1) / 10;
	if (!strcmp(bat_status, "Charging\n") || 
			!strcmp(bat_status, "Full\n"))
		status_emoji = "🔌";
	else
		status_emoji = "🔋";

	sprintf(bat_str, "%c%s %lli%%\x01", color_char, status_emoji,
			bat_level);
	raise(SIGWRITE);
	return;
}

/* BRIGHTNESS */
static void *bright_loop(int signum)
{
	raise(signum);
	return NULL;
}

static void bright_handler(int signum)
{
	static char brt[STRLEN];

	readfile(BRIGHT_PATH, brt);
	sprintf(bright_str, "💡 %d%%", atoi(brt)*100/BRIGHT_MAX);
	raise(SIGWRITE);
	return;
}

/* AUDIO LEVEL */
static void *audio_loop(int signum)
{
	raise(signum);
	return NULL;
}

static void audio_handler(int signum)
{
	static int level;
	FILE *stream;

	stream = popen("amixer -c 1 cget numid=12 |"
			"grep \": values\"", "r");
	fscanf(stream, "  : values=%d", &level);
	pclose(stream);

	sprintf(audio_str, "🔈 -%d dB", 74 - level);
	raise(SIGWRITE);
	return;
}
