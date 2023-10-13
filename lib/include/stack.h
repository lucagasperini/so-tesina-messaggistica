#pragma once

#include <sys/types.h> // size_t

typedef struct _matrix_stack_entry {
        void* data;
        size_t len;
} matrix_stack_entry;

typedef struct _matrix_stack {
        matrix_stack_entry entry;
        struct _matrix_stack* next;
} matrix_stack;

typedef struct _matrix_stack_str {
        char* data;
        struct _matrix_stack_str* next;
} matrix_stack_str;

matrix_stack* matrix_stack_push(matrix_stack* head, void* data, size_t len);
matrix_stack* matrix_stack_pop(matrix_stack* head, matrix_stack_entry* entry);
matrix_stack* matrix_stack_next(matrix_stack* head, matrix_stack_entry* entry);
void matrix_stack_free(matrix_stack* head);

matrix_stack_str* matrix_stack_str_push(matrix_stack_str* head, char* data);
matrix_stack_str* matrix_stack_str_pop(matrix_stack_str* head, char** entry);
matrix_stack_str* matrix_stack_str_next(matrix_stack_str* head, char** entry);
void matrix_stack_str_free(matrix_stack_str* head);