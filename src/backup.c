#include "backup.h"

#define BACKUP_DIR "/tmp/.vfs_sentinel_backups"
#define BACKUP_MAX_SIZE (20 * 1024 * 1024)

static ssize_t fallback_copy(int src_fd, int dest_fd) {
    char buffer[8192];
    ssize_t bytes_read, bytes_written, total_copied = 0;

    lseek(src_fd, 0, SEEK_SET);

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        char *ptr = buffer;
        while (bytes_read > 0) {
            bytes_written = write(dest_fd, ptr, bytes_read);
            if (bytes_written <= 0) {
                return -1;
            }
            bytes_read -= bytes_written;
            ptr += bytes_written;
            total_copied += bytes_written;
        }
    }
    return (bytes_read < 0) ? -1 : total_copied;
}

int backup_file(int src_fd) {
    struct stat st;
    if (fstat(src_fd, &st) < 0) {
        perror("[backup] fstat failed");
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        return -1;
    }
    if (st.st_size > BACKUP_MAX_SIZE) {
        printf("[backup] skipped: file too large (%ld bytes)\n", (long)st.st_size);
        return -1;
    }

    if (mkdir(BACKUP_DIR, 0700) < 0 && errno != EEXIST) {
        perror("[backup] mkdir failed");
        return -1;
    }

    char backup_path[256];
    snprintf(backup_path, sizeof(backup_path), "%s/%lu.bak", BACKUP_DIR, (unsigned long)st.st_ino);

    int dest_fd = open(backup_path, O_CREAT | O_EXCL | O_WRONLY | O_NOFOLLOW, 0600);
    if (dest_fd < 0) {
        if (errno == EEXIST) {
            return 0; // الملف منسوخ سابقاً
        }
        perror("[backup] open(dest) failed");
        return -1;
    }

    off_t offset = 0;
    ssize_t copied = copy_file_range(src_fd, &offset, dest_fd, NULL, st.st_size, 0);

    if (copied < 0) {
        copied = fallback_copy(src_fd, dest_fd);
    }

    if (copied < 0) {
        perror("[backup] copy failed");
        close(dest_fd);
        unlink(backup_path);
        return -1;
    }

    printf("[backup] OK: saved %ld bytes to %s\n", (long)copied, backup_path);
    close(dest_fd);
    return 0;
}