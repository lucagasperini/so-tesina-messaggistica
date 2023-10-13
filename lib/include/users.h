#pragma once

//TODO: This can be moved on server!

#include "stack.h"

#include <stdbool.h> // bool, true, false
#include <sys/types.h> // size_t

bool matrix_users_add(const char* username, const char* userpass);
bool matrix_users_find(const char* username);
bool matrix_users_login(const char* username, const char* userpass);
matrix_stack_str* matrix_users_list(size_t* num);
matrix_stack* matrix_user_inbox(const char* user);