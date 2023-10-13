#pragma once

#include "net.h"
#include "proto.h"
#include <stdbool.h> // bool, true, false

typedef struct _client_thread_info {
        int index;
        int status;
        matrix_connection con;
        char username[MATRIX_USERNAME_MAX_LEN];
} client_thread_info;


void threads_init(int num_threads);
bool threads_start(matrix_connection* con);