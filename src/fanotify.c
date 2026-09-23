#include "fanotify.h"

int init_fanotify(void){
    int fan_fd = fanotify_init(FAN_CLASS_CONTENT | FAN_CLOEXEC, O_RDONLY);
    if (fan_fd == -1){
        perror("[ERROR] fanotify_init failed (Make sure to run with sudo)");
        return -1;
    }
    return fan_fd;
}