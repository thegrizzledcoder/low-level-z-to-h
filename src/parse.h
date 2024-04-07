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
    unsigned int next_id;
};

struct employee_t {
    unsigned int id;
    char name[256];
    char address[256];
    unsigned int hours;
};

struct node_t {
    struct employee_t data;
    struct node_t *next;
};

static const struct employee_t EmptyEmployee;
static const struct node_t EmptyNode;

int create_db_header(struct db_header_t **header_out);
int validate_db_header(int fd, struct db_header_t **header_out);
int read_employees(int fd, const struct db_header_t *, struct node_t **ptr_list_head);
int output_file(int fd, struct db_header_t *, struct node_t **, unsigned short originalCount);
void add_employee(struct db_header_t *, struct node_t **ptr_list_head, char *add_string);
int delete_employee(struct db_header_t *, struct node_t **ptr_list_head, unsigned int id);
void list_employees(struct node_t **);
struct node_t* create_node(struct employee_t*);

#endif //PARSE_H