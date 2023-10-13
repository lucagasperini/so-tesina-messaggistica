#pragma once

#include "file.h"
#include "msg.h"
#include <stdbool.h> // bool

#define MATRIX_SYS_USER_DRAFT        "draft"
#define MATRIX_SYS_USER_SEND         "send"
#define MATRIX_SYS_USER_INBOX        "inbox"

#define MATRIX_SERVER_AUTH_NAME "users.auth"
#define MATRIX_SERVER_ID_NAME "id"

#define MATRIX_SYS_SEPARATOR "/"

char* matrix_sys_home();

char* matrix_sys_main_dir();

matrix_fd matrix_sys_server_auth();

int matrix_sys_server_build(const char* main_dir);
int matrix_sys_client_build(const char* main_dir);

bool matrix_sys_user_dir_create(const char* username);

bool matrix_sys_user_dir(const char* username, char* buf);
bool matrix_sys_user_dir_draft(const char* username, char* buf);
bool matrix_sys_user_dir_inbox(const char* username, char* buf);
bool matrix_sys_user_dir_send(const char* username, char* buf);

matrix_msg_id matrix_sys_server_id();
matrix_msg_id matrix_sys_server_id_add();