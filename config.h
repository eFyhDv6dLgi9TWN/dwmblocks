#ifndef CONFIG_H
#define CONFIG_H 1


#include "dwmblocks.h"

// The filename length of /run/user/$uid/dwmblocks.pid
#define PATH_LEN 256
// The total maximum length of the bar.
#define BAR_LENGTH 256

extern int block_count;
extern Block blocks[];


#endif
