#include "sentinel.h"
#include "canary.h"
#include "fanotify.h"
#include "notify.h"

volatile sig_atomic_t sentinel_stop = 0;

static char global_canary_path[PATH_BUF] = {0};

/* Async-signal-safe: only sets a flag. Cleanup happens in sentinel_run. */
static void handle_shutdown(int sig) {
    (void)sig;
    sentinel_stop = 1;
}

static void install_signal_handlers(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_shutdown;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;                 /* no SA_RESTART: read() returns EINTR so the loop can exit */
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    signal(SIGPIPE, SIG_IGN);
}

static void create_demo_vault(char *target_dir, size_t size) {
    snprintf(target_dir, size, "./demo_vault");
    mkdir(target_dir, 0755);

    char sample_file[PATH_BUF + 32];
    snprintf(sample_file, sizeof(sample_file), "%s/my_passwords.txt", target_dir);
    int fd = open(sample_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd >= 0) {
        const char *msg = "Confidential User Data: 123456\n";
        if (write(fd, msg, strlen(msg)) < 0) perror("write");
        close(fd);
    }
}

int sentinel_run(const char *target_dir_in) {
    char target_dir[PATH_BUF];

    if (geteuid() != 0) {
        fprintf(stderr, "[FATAL] VFS-Sentinel must run as root (fanotify permission events need CAP_SYS_ADMIN).\n"
                        "        Run: sudo ./sentinel <dir>   or install it as a service: sudo make install\n");
        return 1;
    }

    if (target_dir_in != NULL) {
        /* Resolve to an absolute path: systemd starts us in "/" so relative paths would be wrong. */
        if (!realpath(target_dir_in, target_dir)) {
            fprintf(stderr, "[FATAL] Target directory not found: %s (%s)\n", target_dir_in, strerror(errno));
            return 1;
        }
    } else {
        create_demo_vault(target_dir, sizeof(target_dir));
    }

    notify_init();
    install_signal_handlers();

    printf("=====================================================\n");
    printf("         VFS-Sentinel Anti-Ransomware Daemon        \n");
    printf("=====================================================\n");
    printf("[+] Target Directory: %s\n", target_dir);
    printf("[+] PID: %d   (stop: Ctrl+C / systemctl stop vfs-sentinel)\n", getpid());
    printf("=====================================================\n\n");
    fflush(stdout);

    if (create_canary_file(target_dir, global_canary_path, sizeof(global_canary_path)) != 0) {
        fprintf(stderr, "[FATAL] Failed to plant canary trap in: %s\n", target_dir);
        notify_close();
        return 1;
    }

    int fan_fd = init_fanotify();
    if (fan_fd < 0 || mark_directory(fan_fd, target_dir) != 0) {
        if (fan_fd >= 0) close(fan_fd);
        cleanup_canary_file(global_canary_path);
        notify_close();
        return 1;
    }

    char body[PATH_BUF + 64];
    snprintf(body, sizeof(body), "Protection active on %s", target_dir);
    notify_send("low", "VFS-Sentinel started", body);

    start_event_loop(fan_fd);

    /* Normal shutdown path (signal or fanotify error). */
    close(fan_fd);
    cleanup_canary_file(global_canary_path);
    notify_send("normal", "VFS-Sentinel stopped", "Protection is OFF for the monitored folder");
    notify_close();
    return 0;
}
