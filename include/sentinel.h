#ifndef SENTINEL_H
#define SENTINEL_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

#define CANARY_PREFIX "000_canary_"
#define CANARY_SUFFIX ".docx"
#define BUFFER_SIZE 4096
#define PATH_BUF PATH_MAX

/* Set by the signal handler; the event loop checks it and exits cleanly. */
extern volatile sig_atomic_t sentinel_stop;

int sentinel_run(const char *target_dir);

#endif
