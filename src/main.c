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

int main(int argc, char *argv[])
{
    char *file_path = NULL;
    char *add_string = NULL;
    bool new_file = false;
    bool list = false;
    bool delete = false;
    unsigned int id;
    int c;
    int file_desc;
    struct db_header_t *header = NULL;
    struct node_t *employees = NULL;

    while ((c =getopt(argc, argv, "nf:a:ld:")) != -1) {
        switch(c) {
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
                id = strtoul(optarg, NULL, 10);
                break;
            case '?':
                printf("Unknown options -%c\n", c);
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
        if(file_desc == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return STATUS_ERROR;
        }

        if (validate_db_header(file_desc, &header) == STATUS_ERROR) {
            printf("Invalid database file header\n");
            return STATUS_ERROR;
        }
    }

    if (read_employees(file_desc, header, &employees) != STATUS_SUCCESS) {
        printf("Failed to read employees\n");
        return STATUS_ERROR;
    }

    if (add_string) {
        header->count++;
        add_employee(&employees, add_string);
    }

    if (list) {
        list_employees(&employees);
    }

    // if we delete, we need to know the original count
    // of records so that we can truncate the file.
    int originalCount = header->count;

    if (delete) {
        delete_employee(header, &employees, id);
    }

    output_file(file_desc, header, &employees, originalCount);

    return 0;
}
