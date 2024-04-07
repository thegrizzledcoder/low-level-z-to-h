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

int output_file(int fd, struct db_header_t *header, struct node_t **ptr_list_head, const unsigned short originalCount) {
    if (fd < 0) {
        printf("Got a bad file descriptor from the user\n");
        return STATUS_ERROR;
    }

    if (header == NULL) {
        printf("No header defined\n");
        return STATUS_ERROR;
    }

    if (ptr_list_head == NULL) {
        printf("No linked list defined\n");
        return STATUS_ERROR;
    }

    int real_count = header->count;
    unsigned int real_size = (sizeof(struct db_header_t) + sizeof(struct employee_t) * real_count);

    header->magic = htonl(header->magic);
    header->filesize = htonl(real_size);
    header->count = htons(header->count);
    header->version = htons(header->version);

    lseek(fd, 0, SEEK_SET);

    if (!write(fd, header, sizeof(struct db_header_t))) {
        perror("write");
        return STATUS_ERROR;
    }

    struct node_t *temp = *ptr_list_head;
    while(temp != NULL) {
        temp->data.hours = htonl(temp->data.hours);
        temp->data.id  = htonl(temp->data.id);
        if (write(fd, &temp->data, sizeof(struct employee_t)))
            temp = temp->next;
        else
        {
            perror("write");
            return STATUS_ERROR;
        }
    }

    if (originalCount > real_count) {
        if(!ftruncate(fd, real_size)) {
            perror("ftruncate");
            return STATUS_ERROR;
        }
    }

    return STATUS_SUCCESS;
}
int create_db_header(struct db_header_t **header_out) {
    struct db_header_t *header = calloc(1, sizeof(struct db_header_t));
    if (header == NULL) {
        printf("Calloc failed to create db header");
        return STATUS_ERROR;
    }

    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct db_header_t);
    header->next_id = 1;

    *header_out = header;

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct db_header_t **header_out) {
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
    header->next_id = ntohl(header->next_id);

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

    struct stat db_stat = {0};
    fstat(fd, &db_stat);
    if (header->filesize != db_stat.st_size) {
        printf("Corrupted database\n");
        free(header);
        return STATUS_ERROR;
    }

    *header_out = header;

    return STATUS_SUCCESS;
}

struct node_t* create_node(struct employee_t *employee)
{
    struct node_t *node = malloc(sizeof(struct node_t));
    node->data = *employee;
    node->next = NULL;

    return node;
}

int read_employees(const int fd, const struct db_header_t * header, struct node_t **ptr_list_head) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    if (header->count == 0) {
        *ptr_list_head = NULL;
        return STATUS_SUCCESS;
    }

    const int count = header->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL) {
        printf("Malloc failed\n");
        return STATUS_ERROR;
    }

    if(!read(fd, employees, count * sizeof(struct employee_t))){
        perror("read");
        return STATUS_ERROR;
    }

    for(int i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
        employees[i].id = ntohl(employees[i].id);
    }

    struct node_t *head = NULL;
    struct node_t *tail = NULL;
    for (int i = 0; i < count; i++) {
        struct node_t *new_node = create_node(&employees[i]);
        if (head == NULL)
        {
            head = new_node;
        } else {
            tail->next = new_node;
        }
        tail = new_node;
    }

    *ptr_list_head = head;

    free(employees);

    return STATUS_SUCCESS;
}

void add_employee(struct db_header_t *header, struct node_t **ptr_list_head, char *add_string) {
    char *name = strtok(add_string, ",");
    char *addr = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");
    const unsigned int id = header->next_id++;

    printf("%d %s %s %s\n", id, name, addr, hours);

    struct node_t *last = *ptr_list_head;
    struct node_t *new_node = malloc(sizeof(struct node_t));
    struct employee_t new_emp = {0};
    strncpy(new_emp.name, name, sizeof(new_emp.name));
    strncpy(new_emp.address, addr, sizeof(new_emp.address));
    new_emp.hours = strtoul(hours, NULL, 10);
    new_emp.id = id;
    new_node->next = NULL;
    new_node->data = new_emp;

    if (*ptr_list_head == NULL) {
        *ptr_list_head = new_node;
        return;
    }

    while (last->next != NULL) last = last->next;

    last->next = new_node;
}

int delete_employee(struct db_header_t *header, struct node_t **ptr_list_head, const unsigned int id) {
    struct node_t *temp = *ptr_list_head;
    struct node_t *prev = temp;
    // If the employee is HEAD
    if (temp != NULL && temp->data.id == id) {
        *ptr_list_head = temp->next;
        free(temp);
        header->count--;
        return STATUS_SUCCESS;
    }

    // If the employee isn't HEAD
    while (temp != NULL && temp->data.id != id) {
        prev = temp;
        temp = temp->next;
    }

    // If the employee isn't present
    if (temp == NULL || prev == NULL) return STATUS_NOT_FOUND;

    // Remove the node
    prev->next = temp->next;
    header->count--;

    free(temp);

    return STATUS_SUCCESS;
}

void list_employees(struct node_t **ptr_list_head) {
    if(ptr_list_head == NULL) {
        printf("Node list is not initialized\n");
        return;
    }
    struct node_t *temp = *ptr_list_head;
    while (temp != NULL) {
        printf("Employee ID: %d\n", temp->data.id);
        printf("\tName: %s\n", temp->data.name);
        printf("\tAddress: %s\n", temp->data.address);
        printf("\tHours Worked: %d\n", temp->data.hours);
        temp = temp->next;
    }
}
