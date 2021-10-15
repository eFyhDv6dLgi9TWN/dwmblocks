#include "dwmblocks.h"
#include "config.h"


static char status_str[2][BAR_LENGTH];


static inline int get_status()
{
	strcpy(status_str[1], status_str[0]);
	status_str[0][0] = '\0';
	for(int i = 0; i < COUNT(blocks); i++) {
		strcat(status_str[0], " ");
		strcat(status_str[0], blocks[i].str);
		strcat(status_str[0], " ");
        if (i != COUNT(blocks) - 1)
		strcat(status_str[0], "|");
	}
	return strcmp(status_str[0], status_str[1]);
}

// Signal Handlers
static inline void setroot(int signum)
{
	static Display *display;
	int screen;
	Window root;

	if (get_status() == 0)  // Only write out if text has changed.
		return;

	Display *d = XOpenDisplay(NULL);
	if (d)
		display = d;

	screen = DefaultScreen(display);
	root = RootWindow(display, screen);
	XStoreName(display, root, status_str[0]);
	XCloseDisplay(display);

	return;
}

static inline void pstdout(int signum)
{
	if (get_status() == 0)
		return;

	puts(status_str[0]);
	fflush(stdout);
	return;
}

// Thread Function
static inline void *thread_routine(void *arg) {
	Block *block = (Block *) arg;

	return (*(block->main_loop))(block->signum);
}

// Static Functions
static inline void write_pid() {
	int err;
	char *runtime_dir;
	char pid_filename[PATH_LEN];
	FILE *pid_file;

	// Prepare the pid file name.
	runtime_dir = getenv("XDG_RUNTIME_DIR");
	if (!runtime_dir) {
		err = sprintf(pid_filename, "/run/user/%d/dwmblocks.pid",
				getuid());
		if (err < 0) {
			perror("sprintf");
			exit(EXIT_FAILURE);
		}
	} else {
		strcpy(pid_filename, runtime_dir);
		strcat(pid_filename, "/dwmblocks.pid");
	}

	// Write the pid.
	pid_file = fopen(pid_filename, "w");
	if (pid_file == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	err = fprintf(pid_file, "%d", getpid());
	if (err < 0) {
		perror("fprintf");
		exit(EXIT_FAILURE);
	}
	err = fclose(pid_file);
	if (err) {
		perror("fclose");
		exit(EXIT_FAILURE);
	}
}

static inline void setup() {
	int err;
	pthread_t thread_id;
	Block *pBlock = blocks;

	if (signal(SIGWRITE, write_status) == SIG_ERR) {
		perror("signal");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < COUNT(blocks); i++) {
		if (!pBlock->main_loop) break;
		if (signal(pBlock->signum, pBlock->handler) == SIG_ERR) {
			perror("signal");
			exit(EXIT_FAILURE);
		}
		err = pthread_create(&thread_id, NULL, thread_routine, 
				(void *) pBlock);
		if (err) {
			pferror("pthread_create", "Error.");
			exit(EXIT_FAILURE);
		}
		++pBlock;
	}
}

// Main
int main (int argc, char *argv[]) {
	write_status = setroot;
	for (int i = 0; i < argc; i++)
		if (!strcmp(argv[i], "-p")) write_status = pstdout;

	write_pid();
	setup();
	while (true) pause();  // Receiving signals.
}
