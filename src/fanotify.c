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
    if (ret == -1){
        perror("[ERROR] fanotify_mark failed");
        return -1;
    }
    printf("[VFS-Sentinel] Monitoring active on directory: %s\n", dir_path);
    return 0;
}

static void get_filepath_from_fd(int fd, char *path_buffer, size_t size){
    char proc_path[64];
    snprintf(proc_path, sizeof(proc_path), "/proc/self/fd/%d", fd);
    ssize_t len = readlink(proc_path, path_buffer, size - 1);
    if (len != -1){
        path_buffer[len] = '\0';
    } else {
        snprintf(path_buffer, size, "UNKNOWN");
    }

}