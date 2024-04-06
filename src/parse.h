//
// Created by john on 2/14/24.
//

#ifndef PARSE_H
#define PARSE_H

#define HEADER_MAGIC 0x4c4c4144

struct db_header_t {
    unsigned int magic;
    unsigned short version;
    unsigned short count;
    unsigned int filesize;
};

struct employee_t {
    unsigned int id;
    char name[256];
    char address[256];
    unsigned int hours;
};

struct node_t {
    struct employee_t *value;
    struct node_t *next;
};

int create_db_header(struct db_header_t **headerOut);
int validate_db_header(int fd, struct db_header_t **headerOut);
int read_employees(int fd, struct db_header_t *, struct node_t **employees_out);
int output_file(int fd, struct db_header_t *, struct node_t **, unsigned short originalCount);
void add_employee(struct node_t **employees, char *add_string);
int delete_employee(struct db_header_t *, struct node_t **employees, unsigned int id);
void list_employees(struct node_t **);

#endif //PARSE_H