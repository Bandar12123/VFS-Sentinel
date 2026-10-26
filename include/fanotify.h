#ifndef FANOTIFY_H
#define FANOTIFY_H

#include "sentinel.h"
#include "canary.h"

int  init_fanotify(void);
int  mark_directory(int fan_fd, const char *dir_path);
int  mark_tree(int fan_fd, const char *dir_path);   /* dir_path + all subdirectories */
void start_event_loop(int fan_fd);

#endif