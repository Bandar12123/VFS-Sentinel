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


#define CANARY_PREFIX "000_canary_"
#define CANARY_SUFFIX ".docx"
#define BUFFER_SIZE 4096

#include "canary.h"
#include "fanotify.h"

#endif