#include "backup.h"

#define BACKUP_DIR "/tmp/.vfs_sentinel_backups"
#define BACKUP_MAX_SIZE (20 * 1024 * 1024)

int backup_file(int src_fd) {
    struct stat st;
    if (fstat(src_fd, &st) < 0 || S_ISREG(st.st_mode)) {
        return -1;
    }
    if (st.st_size > BACKUP_MAX_SIZE){
        return -1;
    }

    mkdir(BACKUP_DIR, 0700);

    char backup_path[256];
    snprintf(backup_path, sizeof(backup_path), "%s/%lu.bak",
            BACKUP_DIR, (unsigned long)st.st_ino);

    int dest_fd = open(backup_path, O_CREAT | O_EXCL | O_WRONLY | O_NOFOLLOW, 0600);
    if (dest_fd < 0) {
        return (errno == EEXIST) ? 0 : -1;
    }

    off_t offset = 0;
    ssize_t copied = copy_file_range(src_fd, &offset, dest_fd, NULL, st.st_size, 0);
    close(dest_fd);
    return (copied >= 0) ? 0 : -1;
}