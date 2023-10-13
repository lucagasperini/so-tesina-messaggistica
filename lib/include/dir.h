#pragma once

#include "stack.h"
#include <sys/types.h> // size_t, ssize_t

#define MATRIX_DIR_PERMISSION S_IRUSR | S_IWUSR | S_IXUSR

typedef void* matrix_dir;

int matrix_dir_create(const char* dir);
matrix_dir matrix_dir_open(const char* dir);
void matrix_dir_close(matrix_dir dir);
ssize_t matrix_dir_entries_count(matrix_dir dir);
matrix_stack_str* matrix_dir_entries_name(matrix_dir dir);