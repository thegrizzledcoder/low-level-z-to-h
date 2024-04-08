#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage %s -n -f <database file>\n", argv[0]);
    printf("\t-n - create new database file\n");
    printf("\t-f - (required) path to database file\n");
}

void free_linked_list(struct node_t *head) {
    while (head != NULL) {
        struct node_t *temp = head;
        head = head->next;
        free(temp);
    }
}

int main(const int argc, char *argv[]) {
    const char *file_path = NULL;
    char *add_string = NULL;
    bool new_file = false;
    bool list = false;
    bool delete = false;
    unsigned int user_id = 0;
    int char_in;
    int file_desc;
    struct db_header_t *header = {0};
    struct node_t *head = {0};

    while ((char_in = getopt(argc, argv, "nf:a:ld:")) != -1) {
        switch (char_in) {
            case 'f':
                file_path = optarg;
                break;
            case 'n':
                new_file = true;
                break;
            case 'a':
                add_string = optarg;
                break;
            case 'l':
                list = true;
                break;
            case 'd':
                delete = true;
                user_id = strtoul(optarg, NULL, BASE10);
                break;
            case '?':
                printf("Unknown options -%char_in\n", char_in);
                break;
            default:
                print_usage(argv);
                return STATUS_ERROR;
        }
    }

    if (file_path == NULL) {
        printf("Filepath is a required argument\n");
        print_usage(argv);
    }

    if (new_file) {
        file_desc = create_db_file(file_path);
        if (file_desc == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return STATUS_ERROR;
        }

        if (create_db_header(&header) == STATUS_ERROR) {
            printf("Failed to create db header\n");
            return STATUS_ERROR;
        }
    } else {
        file_desc = open_db_file(file_path);
        if (file_desc == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return STATUS_ERROR;
        }

        if (validate_db_header(file_desc, &header) == STATUS_ERROR) {
            printf("Invalid database file header\n");
            return STATUS_ERROR;
        }
    }

    if (read_employees(file_desc, header, &head) != STATUS_SUCCESS) {
        printf("Failed to read head\n");
        return STATUS_ERROR;
    }

    if (add_string) {
        header->count++;
        add_employee(header, &head, add_string);
    }

    if (list) {
        list_employees(&head);
    }

    if (delete) {
        delete_employee(header, &head, user_id);
    }

    output_file(file_desc, header, &head);

    free(header);

    free_linked_list(head);

    return 0;
}
