#ifndef CANARY_H
#define CANARY_H

#include "sentinel.h"

int  create_canary_file(const char *target_dir, char *out_canary_path);

int  is_canary_file(const char *filepath);

void cleanup_canary_file(const char *canary_path);

#endif