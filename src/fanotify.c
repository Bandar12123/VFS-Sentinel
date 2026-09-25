#include "fanotify.h"
#include "backup.h"

int init_fanotify(void) {
    int fan_fd = fanotify_init(FAN_CLASS_CONTENT | FAN_CLOEXEC, O_RDONLY);
    if (fan_fd == -1) {
        perror("[ERROR] fanotify_init failed (Make sure to run with sudo)");
        return -1;
    }
    return fan_fd;
}

int mark_directory(int fan_fd, const char *dir_path) {
    int ret = fanotify_mark(fan_fd,
                            FAN_MARK_ADD,
                            FAN_OPEN_PERM | FAN_EVENT_ON_CHILD,
                            AT_FDCWD,
                            dir_path);
    if (ret == -1) {
        perror("[ERROR] fanotify_mark failed");
        return -1;
    }
    printf("[VFS-Sentinel] Monitoring active on directory: %s\n", dir_path);
    return 0;
}

static void get_filepath_from_fd(int fd, char *path_buffer, size_t size) {
    char proc_path[64];
    snprintf(proc_path, sizeof(proc_path), "/proc/self/fd/%d", fd);
    ssize_t len = readlink(proc_path, path_buffer, size - 1);
    if (len != -1) {
        path_buffer[len] = '\0';
    } else {
        snprintf(path_buffer, size, "UNKNOWN");
    }
}

void start_event_loop(int fan_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t len;
    pid_t my_pid = getpid();

    printf("[VFS-Sentinel] Daemon running.. Listening for VFS events.\n");

    while ((len = read(fan_fd, buffer, sizeof(buffer))) > 0) {
        struct fanotify_event_metadata *metadata;
        metadata = (struct fanotify_event_metadata *)buffer;

        while (FAN_EVENT_OK(metadata, len)) {
            if (metadata->fd < 0) {
                fprintf(stderr, "[WARN] fanotify queue overflow - some events were missed\n");
                metadata = FAN_EVENT_NEXT(metadata, len);
                continue;
            }

            struct fanotify_response response;
            response.fd = metadata->fd;

            if (metadata->pid == my_pid) {
                response.response = FAN_ALLOW;
                if (write(fan_fd, &response, sizeof(response)) < 0) {
                    perror("write response");
                }
                close(metadata->fd);
                metadata = FAN_EVENT_NEXT(metadata, len);
                continue;
            }

            char filepath[512];
            get_filepath_from_fd(metadata->fd, filepath, sizeof(filepath));
            printf("[EVENT] PID %d requested access to:  %s\n", metadata->pid, filepath);

            if (is_canary_file(filepath)) {
                char exe_path[256] = "UNKNOWN";
                char proc_exe_link[64];
                snprintf(proc_exe_link, sizeof(proc_exe_link), "/proc/%d/exe", metadata->pid);
                ssize_t exe_len = readlink(proc_exe_link, exe_path, sizeof(exe_path) - 1);
                if (exe_len > 0) {
                    exe_path[exe_len] = '\0';
                } else {
                    snprintf(exe_path, sizeof(exe_path), "UNKNOWN (process may have already exited)");
                }

                if (strstr(exe_path, "localsearch-extractor") != NULL ||
                    strstr(exe_path, "tracker-miner") != NULL ||
                    strstr(exe_path, "gvfsd") != NULL ||
                    strstr(exe_path, "baloo_file") != NULL) {
                    printf("[VFS-Sentinel] Ignoring trusted system indexer: %s (PID %d)\n", exe_path, metadata->pid);
                    response.response = FAN_ALLOW;
                } else {
                    printf("\n[!  RANSOMWARE ALERT  !]\n");
                    printf("[VFS-Sentinel] Canary trap triggered by PID: %d (program: %s)\n", metadata->pid, exe_path);
                    printf("[ACTION] Terminating process %d with SIGKILL and Blocking access.\n\n", metadata->pid);

                    kill(metadata->pid, SIGKILL);
                    response.response = FAN_DENY;
                }
            } else {
                backup_file(metadata->fd);
                response.response = FAN_ALLOW;
            }

            if (write(fan_fd, &response, sizeof(response)) < 0) {
                perror("write response");
            }

            close(metadata->fd);
            metadata = FAN_EVENT_NEXT(metadata, len);
        }
    }
}