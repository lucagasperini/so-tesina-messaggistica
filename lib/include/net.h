#pragma once

#include "file.h"

#define NET_DEFAULT_PORT 3333u
#define STR_HOST_LEN 16

typedef struct _matrix_connection {
        int fd;
        struct sockaddr_in* net_addr;
} matrix_connection;

int matrix_connect(matrix_connection* con, char dest_host[STR_HOST_LEN], uint16_t dest_port);

void matrix_listen(matrix_connection* con, uint16_t net_port, size_t max_connections);
int matrix_accept(matrix_connection* client, matrix_connection* server);

void matrix_disconnect(matrix_connection* con);

static inline size_t matrix_send(matrix_connection* con, const void* msg, size_t msg_size)
{
        return matrix_write(con->fd, msg, msg_size);
}

static inline size_t matrix_send_blk(matrix_connection* con, const void* msg)
{
        return matrix_write_blk(con->fd, msg);
}

static inline size_t matrix_send_char(matrix_connection* con, char msg)
{
        return matrix_write_char(con->fd, msg);
}

static inline size_t matrix_send_i8(matrix_connection* con, int8_t msg)
{
        return matrix_write_i8(con->fd, msg);
}

static inline size_t matrix_send_i16(matrix_connection* con, int16_t msg)
{
        return matrix_write_i16(con->fd, msg);
}

static inline size_t matrix_send_i32(matrix_connection* con, int32_t msg)
{
        return matrix_write_i32(con->fd, msg);
}

static inline size_t matrix_send_i64(matrix_connection* con, int64_t msg)
{
        return matrix_write_i64(con->fd, msg);
}

static inline size_t matrix_send_u8(matrix_connection* con, uint8_t msg)
{
        return matrix_write_u8(con->fd, msg);
}

static inline size_t matrix_send_u16(matrix_connection* con, uint16_t msg)
{
        return matrix_write_u16(con->fd, msg);
}

static inline size_t matrix_send_u32(matrix_connection* con, uint32_t msg)
{
        return matrix_write_u32(con->fd, msg);
}

static inline size_t matrix_send_u64(matrix_connection* con, uint64_t msg)
{
        return matrix_write_u64(con->fd, msg);
}

static inline size_t matrix_send_line(matrix_connection* con, const char* msg)
{
        return matrix_write_line(con->fd, msg);
}

static inline size_t matrix_send_string(matrix_connection* con, const char* msg)
{
        return matrix_write_string(con->fd, msg);
}

static inline size_t matrix_receive(matrix_connection* con, void* msg, size_t msg_size)
{
        return matrix_read(con->fd, msg, msg_size);
}

static inline size_t matrix_receive_blk(matrix_connection* con, void* blk)
{
        return matrix_read_blk(con->fd, blk);
}

static inline size_t matrix_receive_i8(matrix_connection* con, int8_t* msg)
{
        return matrix_read_i8(con->fd, msg);
}

static inline size_t matrix_receive_i16(matrix_connection* con, int16_t* msg)
{
        return matrix_read_i16(con->fd, msg);
}

static inline size_t matrix_receive_i32(matrix_connection* con, int32_t* msg)
{
        return matrix_read_i32(con->fd, msg);
}

static inline size_t matrix_receive_i64(matrix_connection* con, int64_t* msg)
{
        return matrix_read_i64(con->fd, msg);
}

static inline size_t matrix_receive_u8(matrix_connection* con, uint8_t* msg)
{
        return matrix_read_u8(con->fd, msg);
}

static inline size_t matrix_receive_u16(matrix_connection* con, uint16_t* msg)
{
        return matrix_read_u16(con->fd, msg);
}

static inline size_t matrix_receive_u32(matrix_connection* con, uint32_t* msg)
{
        return matrix_read_u32(con->fd, msg);
}

static inline size_t matrix_receive_u64(matrix_connection* con, uint64_t* msg)
{
        return matrix_read_u64(con->fd, msg);
}

static inline size_t matrix_receive_line(matrix_connection* con, char* msg, size_t msg_size)
{
        return matrix_read_line(con->fd, msg, msg_size);
}

static inline size_t matrix_receive_string(matrix_connection* con, char* msg, size_t msg_size)
{
        return matrix_read_string(con->fd, msg, msg_size);
}

void matrix_copy_connection(matrix_connection* dest, const matrix_connection* src);