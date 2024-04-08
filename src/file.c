//
// Created by john on 3/6/24.
//
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include "file.h"
#include "common.h"

static const mode_t FILE_MODE = 0644;

int create_db_file(const char *filename) {
    int file_desc = open(filename, O_RDONLY);
    if (file_desc != -1) {
        close(file_desc);
        printf("File already exists\n");
        return STATUS_ERROR;
    }

    file_desc = open(filename, O_RDWR | O_CREAT, FILE_MODE);
    if (file_desc == -1) {
        perror("open");
        return STATUS_ERROR;
    }

    return file_desc;
}

int open_db_file(const char *filename) {
    const int file_desc = open(filename, O_RDWR, FILE_MODE);

    if (file_desc == -1) {
        perror("open");
        return STATUS_ERROR;
    }

    return file_desc;
}
