#include "darray.h"

#include <string.h> // memcpy

#define MATRIX_DARRAY_REALLOC_FACTOR 2

void matrix_darray_create(matrix_darray* arr, size_t blk, size_t num)
{
        arr->sz = blk * num;
        arr->blk = blk;
        arr->num = 0;
        MATRIX_MALLOC(arr->mem, arr->sz);
}

void matrix_darray_push(matrix_darray* arr, const void* src)
{
        if(arr->num * arr->blk >= arr->sz) {
                arr->sz *= MATRIX_DARRAY_REALLOC_FACTOR;
                MATRIX_REALLOC(arr->mem, arr->sz);
        }
        memcpy(arr->mem + (arr->num * arr->blk), src, arr->blk);
        arr->num++;
}

void matrix_darray_at(matrix_darray* arr, void* dest, size_t num)
{
        if(num >= arr->num) {
                MATRIX_LOG_ERR("darray doesnt have such capacity [arr_num: %lu, arr_blk: %lu, arr_sz: %lu, num: %lu]",
                        arr->num,
                        arr->blk,
                        arr->sz,
                        num
                );
                return;
        }

        memcpy(dest, arr->mem + (num * arr->blk), arr->blk);
}


void matrix_darray_pop(matrix_darray* arr, void* dest)
{
        if(dest != NULL) {
                matrix_darray_at(arr, dest, arr->num - 1);
        }
        arr->num--;
}

void matrix_darray_free(matrix_darray* arr)
{
        MATRIX_FREE(arr->mem);
        arr->mem = NULL;
        arr->blk = 0;
        arr->num = 0;
        arr->sz = 0;
}