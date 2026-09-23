#include "canary.h"

int create_canary_file(const char *target_dir, char *out_canary_path){
    if (!target_dir || !out_canary_path){
        return -1;
    }

    snprintf(out_canary_path, BUFFER_SIZE, "%s/%s%s", target_dir, CANARY_PREFIX, CANARY_SUFFIX);

    int fd = open(out_canary_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0){
        perror("Failed to create canary file");
        return -1;
    }

    const char dummy_docx_header[] = "PK\x03\x04_VFS_SENTINEL_CANARY_PROTECTED_DATA";
    if(write(fd, dummy_docx_header, sizeof(dummy_docx_header) -1) < 0){
        perror("Failed to write to canary file");
        close(fd);
        return -1;
    }
    close(fd);
    printf("Canary file created at: %s\n", out_canary_path);
    return 0;

}

int is_canary_file(const char *filepath){
    if (filepath == NULL) return 0;
    return (strstr(filepath, CANARY_PREFIX) !=NULL);

}