#include "sentinel.h"

static char global_canary_path[512] = {0};

static void handle_shutdown(int sig) {
    (void)sig;
    printf("\n\n[VFS-Sentinel] Signal received. Shutting down daemon safely...\n");
    cleanup_canary_file(global_canary_path);
    printf("[VFS-Sentinel] Cleaned up and exited successfully.\n");
    exit(0);
}

int sentinel_run(const char *target_dir_in) {
    char target_dir[512];

    if (target_dir_in != NULL) {
        strncpy(target_dir, target_dir_in, sizeof(target_dir) - 1);
        target_dir[sizeof(target_dir) - 1] = '\0';
    } else {
        strncpy(target_dir, "./demo_vault", sizeof(target_dir) - 1);
        target_dir[sizeof(target_dir) - 1] = '\0';

        mkdir(target_dir, 0755);

        char sample_file[512 + 32];
        snprintf(sample_file, sizeof(sample_file), "%s/my_passwords.txt", target_dir);
        int fd = open(sample_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd >= 0) {
            const char *msg = "Confidential User Data: 123456\n";
            if (write(fd, msg, strlen(msg)) < 0) {
                perror("write");
            }
            close(fd);
        }
    }

    printf("=====================================================\n");
    printf("         VFS-Sentinel Anti-Ransomware Daemon        \n");
    printf("=====================================================\n");
    printf("[+] Target Directory: %s\n", target_dir);
    printf("[+] Press Ctrl+C to stop protection.\n");
    printf("=====================================================\n\n");

    signal(SIGINT, handle_shutdown);
    signal(SIGTERM, handle_shutdown);

    if (create_canary_file(target_dir, global_canary_path) != 0) {
        fprintf(stderr, "[FATAL] Failed to plant canary trap in: %s\n", target_dir);
        return 1;
    }

    int fan_fd = init_fanotify();
    if (fan_fd < 0) {
        cleanup_canary_file(global_canary_path);
        return 1;
    }

    if (mark_directory(fan_fd, target_dir) != 0) {
        close(fan_fd);
        cleanup_canary_file(global_canary_path);
        return 1;
    }

    start_event_loop(fan_fd);

    close(fan_fd);
    cleanup_canary_file(global_canary_path);
    return 0;
}
