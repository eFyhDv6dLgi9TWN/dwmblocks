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
/* Temperature */
#define TEMP_COUNT 7
#define TEMP_PATH "/sys/class/thermal/thermal_zone%d/temp"
#define TEMP_MIN 25
#define TEMP_MAX 120
/* Function-like */
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

/* GLOBAL VARIABLES */
static char time_str[STRLEN];
static char date_str[STRLEN];
static char bat_str[STRLEN];
static char bright_str[STRLEN];
static char audio_str[STRLEN];
static char wifi_str[STRLEN];
static char temp_str[STRLEN];

/* GLOBAL FUNCTIONS */
/* Main Loops */
static void *time_loop(int);
static void *bat_loop(int);
static void *bright_loop(int);
static void *audio_loop(int);
static void *wifi_loop(int);
static void *temp_loop(int);
/* Signal Handlers */
static void date_handler(int);
static void time_handler(int);
static void bat_handler(int);
static void bright_handler(int);
static void audio_handler(int);
static void wifi_handler(int);
static void temp_handler(int);

Block blocks[] = {
	{ temp_str,   temp_loop,   40, temp_handler   },
	{ wifi_str,   wifi_loop,   37, wifi_handler   },
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
	fclose(stream);
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
}

static void date_handler(int signum)
{
	sprintf(date_str, "📅 %04d-%02d-%02d %s",
			time_struct->tm_year+1900,
			time_struct->tm_mon+1,
			time_struct->tm_mday,
			wdays[time_struct->tm_wday]);
	raise(SIGWRITE);
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
}

/* WIFI */
static noreturn void *wifi_loop(int signum)
{
	while (true) {
		raise(signum);
		sleep(5);
	}
}

static void wifi_handler(int signum)
{
	static char c, s[STRLEN];
	size_t size;
	FILE *stream;

	stream = popen("netctl-auto list", "r");
	while (true) {
		/* read line */
		size = fread(&c, 1, 1, stream);
		if (!size) goto no_connection;
		if (c == '*') {
			fread(&c, 1, 1, stream);
			/* read the connected wifi name */
			for (int i = 0; i < STRLEN - 1; i++) {
				if (!fread(s + i, 1, 1, stream) ||
				    s[i] == '\n') {
					s[i] = '\0';
					goto write;
				}
			}
		} else do if (!fread(&c, 1, 1, stream)) goto no_connection;
		       while (c != '\n'); /* skip the line */
	}

no_connection:
	memcpy(s, "-", 2);
write:
	pclose(stream);
	sprintf(wifi_str, "📶 %s", s);
	raise(SIGWRITE);
}

/* TEMPERATURE */
static noreturn void *temp_loop(int signum)
{
	while (true) {
		raise(signum);
		sleep(4);
	}
}

static void temp_handler(int signum) {
	int t, t_max = -273;
	static char color, buf[STRLEN], path[PATH_LEN];

	for (int i = 0; i < TEMP_COUNT; i++) {
		sprintf(path, TEMP_PATH, i);
		readfile(path, buf);
		t = atoi(buf) / 1000;
		t_max = max(t_max, t);
	}

	color = (TEMP_MAX - t_max) * 10 / (TEMP_MAX - TEMP_MIN) + '\x03';
	if      (color < '\x03') color = '\x03';
	else if (color > '\x12') color = '\x12';
	sprintf(temp_str, "%c\xf0\x9f\x8c\xa1 %d\xe2\x84\x83\x01",
		color, t_max);
}
