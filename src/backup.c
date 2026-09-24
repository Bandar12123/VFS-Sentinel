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
}