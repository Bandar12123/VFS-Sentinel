#include "fanotify.h"

int init_fanotify(void){
    int fan_fd = fanotify_init(FAN_CLASS_CONTENT | FAN_CLOEXEC, O_RDONLY);
    if (fan_fd == -1){
        perror("[ERROR] fanotify_init failed (Make sure to run with sudo)");
        return -1;
    }
    return fan_fd;
}

int mark_directory(int fan_fd, const char *dir_path){
    int ret = fanotify_mark(fan_fd,
                            FAN_MARK_ADD,
                            FAN_OPEN_PERM | FAN_EVENT_ON_CHILD,
                            AT_FDCWD,
                            dir_path);
    if (ret == 1){
        perror("[ERROR] fanotify_mark failed");
        return -1;
    }
    printf("[VFS-Sentinel] Monitoring active on directory: %s\n", dir_path);
    return 0;
}