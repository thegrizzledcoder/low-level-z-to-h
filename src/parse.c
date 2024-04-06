//
// Created by john on 3/7/24.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "parse.h"

int output_file(int fd, struct dbheader_t *header, struct employee_t *employees, unsigned short originalCount) {
    if (fd < 0) {
        printf("Got a bad file descriptor from the user\n");
        return STATUS_ERROR;
    }

    int realcount = header->count;
    unsigned int realsize = (sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);

    header->magic = htonl(header->magic);
    header->filesize = htonl(realsize);
    header->count = htons(header->count);
    header->version = htons(header->version);

    lseek(fd, 0, SEEK_SET);

    write(fd, header, sizeof(struct dbheader_t));

    int i;
    for (i=0; i < realcount; ++i) {
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

    if (originalCount > realcount) {
        ftruncate(fd, realsize);
    }

    return STATUS_SUCCESS;
}
int create_db_header(int fd, struct dbheader_t **headerOut) {
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Calloc failed to create db header");
        return STATUS_ERROR;
    }

    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Calloc failed to create a db header\n");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    if (header->magic != HEADER_MAGIC) {
        printf("Invalid header magic\n");
        free(header);
        return STATUS_ERROR;
    }

    if (header->version != 1) {
        printf("Invalid header version\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (header->filesize != dbstat.st_size) {
        printf("Corrupted database\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;

    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t * header, struct employee_t **employeesout) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    int count = header->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL) {
        printf("Malloc failed\n");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(struct employee_t));

    int i = 0;
    for (; i < count; ++i) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesout = employees;

    return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t *header, struct employee_t *employees, char *addstring) {
    char *name = strtok(addstring, ",");
    char *addr = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");

    printf("%s %s %s\n", name, addr, hours);

    strncpy(employees[header->count-1].name, name, sizeof(employees[header->count-1].name));
    strncpy(employees[header->count-1].address, addr, sizeof(employees[header->count-1].address));
    employees[header->count-1].hours = atoi(hours);

    return STATUS_SUCCESS;
}

struct employee_t* delete_employee(struct dbheader_t *header, struct employee_t *employees, char *toRemove) {

    unsigned short i,count = 0;

    for (i = 0; i < header->count; ++i) {
        if(strcmp(employees[i].name, toRemove) == 0) {
            count++;
        }
        printf("Employee %s\n", employees[i].name);
    }

    if (count == 0) {
        printf("Employee %s not found\n", toRemove);
        return employees;
    }

    struct employee_t *newEmployees = calloc(header->count - count, sizeof(struct employee_t));
    int k = 0;
    for (i = 0; i < header->count; i++) {
        if(strcmp(employees[i].name, toRemove) != 0) {
            newEmployees[k++] = employees[i];
        }
    }

    header->count -= count;
    free(employees);
    return newEmployees;
}

void list_employees(struct dbheader_t *header, struct employee_t *employees) {
    int i = 0;
    for(; i < header->count; ++i) {
        printf("Employee %d\n", i);
        printf("\tName: %s\n", employees[i].name);
        printf("\tAddress: %s\n", employees[i].address);
        printf("\tHours Worked: %d\n", employees[i].hours);
    }
}
