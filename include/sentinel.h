#ifndef SENTINEL_H
#define SENTINEL_H

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


#define CANARY_PREFIX "000_canary_"
#define CANARY_SUFFIX ".docx"
#define BUFFER_SIZE 4096

#endif