#pragma once

#include "net.h"
#include <stdbool.h> // bool, true, false
#include "threads.h"

typedef int8_t matrix_request_result;

bool request_start_server(client_thread_info* tinfo);

void request_loop(client_thread_info* tinfo);
matrix_request_result request_put(matrix_connection* con, const char* username, header_size_t sz);
matrix_request_result request_users(matrix_connection* con);
matrix_request_result request_inbox(matrix_connection* con, const char* username);
matrix_request_result request_get(matrix_connection* con, const char* username);
matrix_request_result request_del(matrix_connection* con, const char* username);