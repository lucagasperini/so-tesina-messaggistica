#include "mem.h"
#include "log.h"

#include <string.h> // memcpy, strncpy

void matrix_pack(void* dest, size_t* index, const void* src, size_t len)
{
        MATRIX_ASSERT_VALID_MEM(dest)
        MATRIX_ASSERT_VALID_MEM(src)

        if(index != NULL) {
                memcpy(dest + *index, src, len);
                *index += len;
        } else {
                memcpy(dest, src, len);
        }
}


void matrix_pack_str(void* dest, size_t* index, const char* src)
{
        MATRIX_ASSERT_VALID_MEM(dest)
        MATRIX_ASSERT_VALID_MEM(src)

        matrix_len len = strlen(src) + 1;
        matrix_pack_u32(dest, index, len);
        if(index != NULL) {
                strncpy(dest + *index, src, len);
                *index += len;
        } else {
                strncpy(dest + sizeof(matrix_len), src, len);
        }
}

void matrix_unpack(const void* src, size_t* index, void* dest, size_t len)
{
        MATRIX_ASSERT_VALID_MEM(dest)
        MATRIX_ASSERT_VALID_MEM(src)

        if(index != NULL) {
                memcpy(dest, src + *index, len);
                *index += len;
        } else {
                memcpy(dest, src, len);
        }
}

void matrix_unpack_str(const void* src, size_t* index, char** dest)
{
        MATRIX_ASSERT_VALID_MEM(dest)
        MATRIX_ASSERT_VALID_MEM(src)

        matrix_len len;
        matrix_unpack_u32(src, index, &len);
        MATRIX_MALLOC(*dest, len);

        if(index != NULL) {
                strncpy(*dest, src + *index, len);
                *index += len;
        } else {
                strncpy(*dest, src + sizeof(matrix_len), len);
        }
}

bool matrix_unpack_buf(const void* src, size_t* index, char* buf, size_t buf_sz)
{
        MATRIX_ASSERT_VALID_MEM(buf)
        MATRIX_ASSERT_VALID_MEM(src)

        matrix_len len;
        matrix_unpack_u32(src, index, &len);
        if(buf_sz < len) {
                MATRIX_LOG_ERR("Cannot unpack on buffer [buf_sz: %lu, len: %lu]", buf_sz, len);
                return false;
        }

        if(index != NULL) {
                strncpy(buf, src + *index, len);
                *index += len;
                return true;
        } else {
                strncpy(buf, src + sizeof(matrix_len), len);
        }
}