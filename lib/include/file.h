#pragma once

#include "log.h"

#include <sys/types.h> // size_t, ssize_t
#include <stdint.h> // int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t
#include <stdbool.h>

#define MATRIX_DEFAULT_FILE_PERMISSION S_IRUSR | S_IWUSR
#define MATRIX_DEFAULT_FILE_FLAGS O_CREAT | O_RDWR
#define MATRIX_PATH_LEN 512
#define MATRIX_BLOCK 1024

#define MATRIX_ASSERT_VALID_FD(fd) MATRIX_ASSERT(fd > 0, "Try to access through unvalid file descriptor!")

typedef int matrix_fd;
typedef long matrix_offset;

// TODO: Use a better way to handle open flags
matrix_fd matrix_file_open_oflag_perm(const char* file, int oflag, int perm);
matrix_fd matrix_file_open_oflag(const char* file, int oflag);
matrix_fd matrix_file_open_new(const char* file);
matrix_fd matrix_file_open(const char* file);

matrix_fd matrix_file_open_ro(const char* file);
matrix_fd matrix_file_open_wo(const char* file);
matrix_fd matrix_file_open_wot(const char* file);

bool matrix_file_copy(const char* src_path, const char* dest_path);
bool matrix_file_copy_fd(matrix_fd src_fd, const char* dest_path);

size_t matrix_read(matrix_fd fd, void* buf, size_t sz);

static inline size_t matrix_read_blk(matrix_fd fd, void* buf)
{
        return matrix_read(fd, buf, MATRIX_BLOCK);
}

static inline size_t matrix_read_char(matrix_fd fd, char* buf)
{
        return matrix_read(fd, buf, sizeof(char));
}

static inline size_t matrix_read_i8(matrix_fd fd, int8_t* buf)
{
        return matrix_read(fd, buf, sizeof(int8_t));
}

static inline size_t matrix_read_i16(matrix_fd fd, int16_t* buf)
{
        return matrix_read(fd, buf, sizeof(int16_t));
}

static inline size_t matrix_read_i32(matrix_fd fd, int32_t* buf)
{
        return matrix_read(fd, buf, sizeof(int32_t));
}

static inline size_t matrix_read_i64(matrix_fd fd, int64_t* buf)
{
        return matrix_read(fd, buf, sizeof(int64_t));
}

static inline size_t matrix_read_u8(matrix_fd fd, uint8_t* buf)
{
        return matrix_read(fd, buf, sizeof(uint8_t));
}

static inline size_t matrix_read_u16(matrix_fd fd, uint16_t* buf)
{
        return matrix_read(fd, buf, sizeof(uint16_t));
}

static inline size_t matrix_read_u32(matrix_fd fd, uint32_t* buf)
{
        return matrix_read(fd, buf, sizeof(uint32_t));
}

static inline size_t matrix_read_u64(matrix_fd fd, uint64_t* buf)
{
        return matrix_read(fd, buf, sizeof(uint64_t));
}

size_t matrix_read_line(matrix_fd fd, char* buf, size_t sz);
size_t matrix_read_string(matrix_fd fd, char* buf, size_t sz);

size_t matrix_write(matrix_fd fd, const void* buf, size_t sz);

static inline size_t matrix_write_blk(matrix_fd fd, const void* buf) 
{
        return matrix_write(fd, buf, MATRIX_BLOCK);
}

static inline size_t matrix_write_char(matrix_fd fd, char buf)
{
        return matrix_write(fd, &buf, sizeof(char));
}

static inline size_t matrix_write_i8(matrix_fd fd, int8_t buf)
{
        return matrix_write(fd, &buf, sizeof(int8_t));
}

static inline size_t matrix_write_i16(matrix_fd fd, int16_t buf)
{
        return matrix_write(fd, &buf, sizeof(int16_t));
}

static inline size_t matrix_write_i32(matrix_fd fd, int32_t buf)
{
        return matrix_write(fd, &buf, sizeof(int32_t));
}

static inline size_t matrix_write_i64(matrix_fd fd, int64_t buf)
{
        return matrix_write(fd, &buf, sizeof(int64_t));
}

static inline size_t matrix_write_u8(matrix_fd fd, uint8_t buf)
{
        return matrix_write(fd, &buf, sizeof(uint8_t));
}

static inline size_t matrix_write_u16(matrix_fd fd, uint16_t buf)
{
        return matrix_write(fd, &buf, sizeof(uint16_t));
}

static inline size_t matrix_write_u32(matrix_fd fd, uint32_t buf)
{
        return matrix_write(fd, &buf, sizeof(uint32_t));
}

static inline size_t matrix_write_u64(matrix_fd fd, uint64_t buf)
{
        return matrix_write(fd, &buf, sizeof(uint64_t));
}

size_t matrix_write_stdout(const void* mem, size_t sz);
size_t matrix_write_stderr(const void* mem, size_t sz);

size_t matrix_write_line(matrix_fd fd, const char* buf);
size_t matrix_write_string(matrix_fd fd, const char* buf);

matrix_offset matrix_file_seek(matrix_fd fd, matrix_offset offset, int mode);
matrix_offset matrix_file_seek_start(matrix_fd fd, matrix_offset offset);
matrix_offset matrix_file_seek_current(matrix_fd fd, matrix_offset offset);
matrix_offset matrix_file_seek_end(matrix_fd fd, matrix_offset offset);

static inline void matrix_file_reset(matrix_fd fd)
{
        MATRIX_ASSERT_VALID_FD(fd)
        matrix_file_seek_start(fd, 0);
}

void matrix_file_sync(matrix_fd fd);

static inline void matrix_file_reset_sync(matrix_fd fd)
{
        matrix_file_sync(fd);
        matrix_file_reset(fd);
}

void matrix_file_close(matrix_fd fd);
void matrix_file_truncate(matrix_fd fd, size_t offset);
static inline void matrix_file_clear(matrix_fd fd)
{
        matrix_file_truncate(fd, 0);
}

static inline void matrix_file_clear_close(matrix_fd fd)
{
        matrix_file_clear(fd);
        matrix_file_close(fd);
}

static inline size_t matrix_file_size(matrix_fd fd)
{
        matrix_offset start = matrix_file_seek_start(fd, 0);
        matrix_offset end = matrix_file_seek_end(fd, 0);
        matrix_file_seek_start(fd, 0);
        return end - start;
}

static inline bool matrix_file_is_empty(matrix_fd fd)
{
        // TODO: Replace with matrix_file_size
        matrix_offset start = matrix_file_seek_start(fd, 0);
        matrix_offset end = matrix_file_seek_end(fd, 0);
        matrix_file_seek_start(fd, 0);
        return (end - start) == 0;
}

bool matrix_file_parse_name(const char* fullpath, char* buf);

bool matrix_file_delete(const char* fullpath);