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

int output_file(int fd, struct db_header_t *header, struct node_t **employees, unsigned short originalCount) {
    if (fd < 0) {
        printf("Got a bad file descriptor from the user\n");
        return STATUS_ERROR;
    }

    int real_count = header->count;
    unsigned int real_size = (sizeof(struct db_header_t) + sizeof(struct employee_t) * real_count);

    header->magic = htonl(header->magic);
    header->filesize = htonl(real_size);
    header->count = htons(header->count);
    header->version = htons(header->version);

    lseek(fd, 0, SEEK_SET);

    write(fd, header, sizeof(struct db_header_t));

    struct node_t *temp = *employees;
    while(temp != NULL) {
        temp->value->hours = htonl(temp->value->hours);
        temp->value->id  = htonl(temp->value->id);
        write(fd, temp->value, sizeof(struct employee_t));
        temp = temp->next;
    }

    if (originalCount > real_count) {
        ftruncate(fd, real_size);
    }

    return STATUS_SUCCESS;
}
int create_db_header(struct db_header_t **headerOut) {
    struct db_header_t *header = calloc(1, sizeof(struct db_header_t));
    if (header == NULL) {
        printf("Calloc failed to create db header");
        return STATUS_ERROR;
    }

    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct db_header_t);

    *headerOut = header;

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct db_header_t **headerOut) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    struct db_header_t *header = calloc(1, sizeof(struct db_header_t));
    if (header == NULL) {
        printf("Calloc failed to create a db header\n");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct db_header_t)) != sizeof(struct db_header_t)) {
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

int read_employees(int fd, struct db_header_t * header, struct node_t **employees_out) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    if (header->count == 0) {
        *employees_out = NULL;
        return STATUS_SUCCESS;
    }

    int count = header->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL) {
        printf("Malloc failed\n");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(struct employee_t));

    for(int i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
        employees[i].id = ntohl(employees[i].id);
    }

    struct node_t *head = (struct node_t*)malloc(sizeof(struct node_t));
    struct node_t *temp = head;
    for (int i = 0; i < count; i++) {
        temp->value = &employees[i];
        if(i < (count-1)) {
            temp->next = (struct node_t*)malloc(sizeof(struct node_t));
        }
        temp = temp->next;
    }

    *employees_out = head;

    return STATUS_SUCCESS;
}

void add_employee(struct node_t **employees, char *add_string) {
    char *id = strtok(add_string, ",");
    char *name = strtok(NULL, ",");
    char *addr = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");
    char *endPtr;
    printf("%s %s %s %s\n", id, name, addr, hours);

    struct node_t *new_node;
    struct node_t *last = *employees;
    new_node = (struct node_t*)malloc(sizeof(struct node_t));
    struct employee_t *new_emp = (struct employee_t*)malloc(sizeof(struct employee_t));
    strncpy(new_emp->name, name, sizeof(new_emp->name));
    strncpy(new_emp->address, addr, sizeof(new_emp->address));
    new_emp->hours = strtol(hours, NULL, 10);
    new_emp->id = strtol(id, &endPtr, 10);
    new_node->next = NULL;
    new_node->value = new_emp;

    if (*employees == NULL) {
        *employees = new_node;
        return;
    }

    while (last->next != NULL) last = last->next;

    last->next = new_node;
}

int delete_employee(struct db_header_t *header, struct node_t **employees, unsigned int id) {
    struct node_t *prev, *temp = *employees;
    // If the employee is HEAD
    if (temp != NULL && temp->value->id == id) {
        *employees = temp->next;
        free(temp->value);
        free(temp);
        header->count--;
        return STATUS_SUCCESS;
    }

    // If the employee isn't HEAD
    while (temp != NULL && temp->value->id != id) {
        prev = temp;
        temp = temp->next;
    }

    // If the employee isn't present
    if (temp == NULL) return STATUS_NOT_FOUND;

    // Remove the node
    prev->next = temp->next;
    header->count--;
    free(temp->value);
    free(temp);

    return STATUS_SUCCESS;
}

void list_employees(struct node_t **employees) {
    struct node_t *temp = *employees;
    while (temp != NULL) {
        printf("Employee ID: %d\n", temp->value->id);
        printf("\tName: %s\n", temp->value->name);
        printf("\tAddress: %s\n", temp->value->address);
        printf("\tHours Worked: %d\n", temp->value->hours);
        temp = temp->next;
    }
}
