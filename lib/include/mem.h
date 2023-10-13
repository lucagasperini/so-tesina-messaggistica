#pragma once

#include <sys/types.h> // size_t
#include <stdint.h> // int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t
#include <stdbool.h>

#define MATRIX_ASSERT_VALID_MEM(ptr) MATRIX_ASSERT(ptr != 0, "Try to access to nullptr!")

typedef uint32_t matrix_len;

void matrix_pack(void* dest, size_t* index, const void* src, size_t len);

static inline void matrix_pack_u8(void* dest, size_t* index, uint8_t val)
{
        matrix_pack(dest, index, &val, sizeof(uint8_t));
}
static inline void matrix_pack_u16(void* dest, size_t* index, uint16_t val)
{
        matrix_pack(dest, index, &val, sizeof(uint16_t));
}

static inline void matrix_pack_u32(void* dest, size_t* index, uint32_t val)
{
        matrix_pack(dest, index, &val, sizeof(uint32_t));
}
static inline void matrix_pack_u64(void* dest, size_t* index, uint64_t val)
{
        matrix_pack(dest, index, &val, sizeof(uint64_t));
}

static inline void matrix_pack_i8(void* dest, size_t* index, int8_t val)
{
        matrix_pack(dest, index, &val, sizeof(int8_t));
}
static inline void matrix_pack_i16(void* dest, size_t* index, int16_t val)
{
        matrix_pack(dest, index, &val, sizeof(int16_t));
}
static inline void matrix_pack_i32(void* dest, size_t* index, int32_t val)
{
        matrix_pack(dest, index, &val, sizeof(int32_t));
}
static inline void matrix_pack_i64(void* dest, size_t* index, int64_t val)
{
        matrix_pack(dest, index, &val, sizeof(int64_t));
}

void matrix_pack_str(void* dest, size_t* index, const char* src);

void matrix_unpack(const void* src, size_t* index, void* dest, size_t len);

static inline void matrix_unpack_u8(const void* src, size_t* index, uint8_t* val)
{
        matrix_unpack(src, index, val, sizeof(uint8_t));
}
static inline void matrix_unpack_u16(const void* src, size_t* index, uint16_t* val)
{
        matrix_unpack(src, index, val, sizeof(uint16_t));
}
static inline void matrix_unpack_u32(const void* src, size_t* index, uint32_t* val)
{
        matrix_unpack(src, index, val, sizeof(uint32_t));
}
static inline void matrix_unpack_u64(const void* src, size_t* index, uint64_t* val)
{
        matrix_unpack(src, index, val, sizeof(uint64_t));
}

static inline void matrix_unpack_i8(const void* src, size_t* index, int8_t* val)
{
        matrix_unpack(src, index, val, sizeof(int8_t));
}
static inline void matrix_unpack_i16(const void* src, size_t* index, int16_t* val)
{
        matrix_unpack(src, index, val, sizeof(int16_t));
}
static inline void matrix_unpack_i32(const void* src, size_t* index, int32_t* val)
{
        matrix_unpack(src, index, val, sizeof(int32_t));
}
static inline void matrix_unpack_i64(const void* src, size_t* index, int64_t* val)
{
        matrix_unpack(src, index, val, sizeof(int64_t));
}

void matrix_unpack_str(const void* src, size_t* index, char** dest);
bool matrix_unpack_buf(const void* src, size_t* index, char* buf, size_t buf_sz);