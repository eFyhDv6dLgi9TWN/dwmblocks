#ifndef CONFIG_H
#define CONFIG_H 1


#define PATH_LEN 256
#define STRLEN 64
#define BAR_LENGTH 256
#define FLASH_RATE 500  // In microseconds.

#include <time.h>
#include "dwmblocks.h"
#include "timedate.h"
#include "battery.h"
#include "audio.h"
#include "bright.h"


Block blocks[] = {
	{ audio_str,  audio_loop,  38, audio_handler  },
	{ bright_str, bright_loop, 39, bright_handler },
	{ bat_str,    bat_loop,    34, bat_handler    },
	{ time_str,   time_loop,   35, time_handler   },
	{ date_str,   NULL,        36, date_handler   },
};


#endif
