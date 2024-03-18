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
    char *filepath = NULL;
    char *addstring = NULL;
    bool newfile = false;
    bool list = false;
    int c;
    int file_desc = -1;
    struct dbheader_t *header = NULL;
    struct employee_t *employees = NULL;

    while ((c =getopt(argc, argv, "nf:a:l"))!= -1) {
        switch(c) {
            case 'f':
                filepath = optarg;
                break;
            case 'n':
                newfile = true;
                break;
            case 'a':
                addstring = optarg;
                break;
            case 'l':
                list = true;
                break;
            case '?':
                printf("Unknown options -%c\n", c);
                break;
            default:
                return STATUS_ERROR;
        }
    }

    if (filepath == NULL) {
        printf("Filepath is a required argument\n");
        print_usage(argv);
    }

    if (newfile) {
        file_desc = create_db_file(filepath);
        if (file_desc == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return STATUS_ERROR;
        }

        if (create_db_header(file_desc, &header) == STATUS_ERROR) {
            printf("Failed to create db header\n");
            return STATUS_ERROR;
        }
    } else {
        file_desc = open_db_file(filepath);
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

    if (addstring) {
        header->count++;
        employees = realloc(employees, header->count*(sizeof(struct employee_t)));
        add_employee(header, employees, addstring);
    }

    if (list) {
        list_employees(header, employees);
    }
    output_file(file_desc, header, employees);

    return 0;
}
