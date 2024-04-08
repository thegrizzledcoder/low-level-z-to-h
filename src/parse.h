//
// Created by john on 2/14/24.
//

#ifndef PARSE_H
#define PARSE_H

#define HEADER_MAGIC 0x4c4c4144
#define MAX_STR_LEN 256

struct db_header_t {
    unsigned int magic;
    unsigned short version;
    unsigned short count;
    unsigned int filesize;
    unsigned int next_id;
};

struct employee_t {
    unsigned int id;
    char name[MAX_STR_LEN];
    char address[MAX_STR_LEN];
    unsigned int hours;
};

struct node_t {
    struct employee_t data;
    struct node_t *next;
};

int create_db_header(struct db_header_t **header_out);

int validate_db_header(int file_desc, struct db_header_t **header_out);

int read_employees(int file_desc, const struct db_header_t *, struct node_t **p_head);

int output_file(int file_desc, struct db_header_t *, struct node_t **);

void add_employee(struct db_header_t *, struct node_t **p_head, char *add_string);

int delete_employee(struct db_header_t *, struct node_t **p_head, unsigned int user_id);

void list_employees(struct node_t **);

struct node_t *create_node(struct employee_t *);

#endif //PARSE_H