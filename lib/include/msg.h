#pragma once

#include "net.h"

typedef uint32_t matrix_msg_id;

typedef struct _matrix_msg_info {
        matrix_msg_id id;
        char* sender;
        char* destination;
        char* subject;
} matrix_msg_info;

matrix_fd matrix_msg_create_draft(const char* sender, const char* dest, const char* subject);
matrix_fd matrix_msg_create_file(const char* dir, const char* sender, const char* dest, const char* subject, matrix_msg_id* id);

bool matrix_msg_create(const char* sender, const char* dest, const char* subject, const char* text);

int matrix_msg_append(matrix_fd fd, const char* text);
int matrix_msg_append_field(matrix_fd fd, const char* label, const char* value);

matrix_fd matrix_msg_receive_file(matrix_connection* con, matrix_msg_id id, const char* username);
bool matrix_msg_send_file(matrix_connection* con, matrix_msg_id id, const char* username);
bool matrix_msg_delete_file(matrix_msg_id id, const char* username);

bool matrix_msg_info_file(const char* filepath, matrix_msg_info* info);
bool matrix_msg_info_inbox(const char* user, const char* file, matrix_msg_info* info);

