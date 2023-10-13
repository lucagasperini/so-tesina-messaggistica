#pragma once

#include "log.h"

// dynamic arrays
typedef struct _matrix_darray {
        // memory pointer
        void* mem;
        // size of memory allocated
        size_t sz;
        // size of a block
        size_t blk;
        // number of element pushed
        size_t num;
} matrix_darray;

void matrix_darray_create(matrix_darray* arr, size_t blk, size_t num);
void matrix_darray_push(matrix_darray* arr, const void* src);
void matrix_darray_at(matrix_darray* arr, void* dest, size_t num);
void matrix_darray_pop(matrix_darray* arr, void* dest);
void matrix_darray_free(matrix_darray* arr);

static inline void* matrix_darray_ptr(matrix_darray* arr) {
        return arr->mem;
}
static inline size_t matrix_darray_alloc_sz(matrix_darray* arr) {
        return arr->sz;
}
static inline size_t matrix_darray_used_sz(matrix_darray* arr) {
        return arr->blk * arr->num;
}