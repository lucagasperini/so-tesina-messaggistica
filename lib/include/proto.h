#pragma once

#include "stack.h"
#include "net.h"
#include <stdbool.h>
#include "msg.h"

#define MATRIX_PROTO_MAGIC              2585757991u

#define MATRIX_PROTO_RESET_CLIENT       0x00u
#define MATRIX_PROTO_RESET_SERVER       0x01u

#define MATRIX_PROTO_CONTROL            0x02u
#define MATRIX_PROTO_ACK                0x03u

#define MATRIX_PROTO_CLIENT_PUT         0x04u
#define MATRIX_PROTO_CLIENT_GET         0x05u
#define MATRIX_PROTO_CLIENT_DEL         0x06u
#define MATRIX_PROTO_CLIENT_INBOX       0x07u
#define MATRIX_PROTO_CLIENT_USERS       0x08u

#define MATRIX_PROTO_SERVER_PUT         0x09u
#define MATRIX_PROTO_SERVER_GET         0x0Au
#define MATRIX_PROTO_SERVER_DEL         0x0Bu
#define MATRIX_PROTO_SERVER_INBOX       0x0Cu
#define MATRIX_PROTO_SERVER_USERS       0x0Du

#define MATRIX_PROTO_OP_MAX             0x0Du

static const char* MATRIX_PROTO_OP_STRING_ARRAY[MATRIX_PROTO_OP_MAX + 1] = {
        "MATRIX_PROTO_RESET_CLIENT",
        "MATRIX_PROTO_RESET_SERVER",

        "MATRIX_PROTO_CONTROL",
        "MATRIX_PROTO_ACK",

        "MATRIX_PROTO_CLIENT_PUT",
        "MATRIX_PROTO_CLIENT_GET",
        "MATRIX_PROTO_CLIENT_DEL",
        "MATRIX_PROTO_CLIENT_INBOX",
        "MATRIX_PROTO_CLIENT_USERS",

        "MATRIX_PROTO_SERVER_PUT",
        "MATRIX_PROTO_SERVER_GET",
        "MATRIX_PROTO_SERVER_DEL",
        "MATRIX_PROTO_SERVER_INBOX",
        "MATRIX_PROTO_SERVER_USERS"
};

#define MATRIX_PROTO_OP_STRING(var) MATRIX_PROTO_OP_STRING_ARRAY[var]

#define MATRIX_PROTO_CODE_OK 0x0
#define MATRIX_PROTO_CODE_NOTFOUND 0x1
#define MATRIX_PROTO_CODE_INTERR 0x2
#define MATRIX_PROTO_CODE_INVPAYLOAD 0x3

#define MATRIX_PROTO_CODE_MAX             0x3

static const char* MATRIX_PROTO_CODE_STRING_ARRAY[MATRIX_PROTO_CODE_MAX + 1] = {
        "MATRIX_PROTO_CODE_OK",
        "MATRIX_PROTO_CODE_NOTFOUND",

        "MATRIX_PROTO_CODE_INTERR",
        "MATRIX_PROTO_CODE_INVPAYLOAD",
};

#define MATRIX_PROTO_CODE_STRING(var) MATRIX_PROTO_CODE_STRING_ARRAY[var]

typedef uint64_t header_size_t;
typedef uint8_t header_op_t;
typedef uint32_t header_magic_t;

typedef struct _matrix_proto_header {
        header_magic_t magic;
        header_op_t op;
        header_size_t sz;
        //uint8_t dontcare;
} matrix_proto_header;

#define MATRIX_USERNAME_MAX_LEN 64
#define MATRIX_USERPASS_MAX_LEN 64
#define MATRIX_SUBJECT_MAX_LEN 256
#define MATRIX_PAYLOAD_LIST_OBJ ((sizeof(uint32_t) * 4) + (MATRIX_USERNAME_MAX_LEN * 2) + MATRIX_SUBJECT_MAX_LEN)

#define MATRIX_PROTO_CODE_AUTH_OK 0x1
#define MATRIX_PROTO_CODE_AUTH_FAIL 0x0

bool matrix_proto_create_header(matrix_proto_header* header, header_op_t op, header_size_t sz);

bool matrix_proto_send_header(matrix_connection* con, matrix_proto_header* header);
void matrix_proto_send_payload(matrix_connection* con, const char* payload, size_t sz);
bool matrix_proto_receive_header(matrix_connection* con, matrix_proto_header* header);
void matrix_proto_receive_payload(matrix_connection* con, char* payload, size_t sz);

bool matrix_proto_handshake_client(matrix_connection* con);
bool matrix_proto_handshake_server(matrix_connection* con);

bool matrix_proto_auth_client(matrix_connection* con, const char* username, const char* userpass);
bool matrix_proto_auth_request_server(matrix_connection* con, char* username, char* userpass);
bool matrix_proto_auth_response_server(matrix_connection* con, uint8_t code);

int8_t matrix_proto_put_client(matrix_connection* con, const char* dest, const char* subject, matrix_fd filetext);
bool matrix_proto_put_server(matrix_connection* con, uint8_t code);

matrix_stack_str* matrix_proto_users_client(matrix_connection* con);
bool matrix_proto_users_server(matrix_connection* con, matrix_stack_str* userlist, size_t num_users);

matrix_stack* matrix_proto_inbox_client(matrix_connection* con);
bool matrix_proto_inbox_server(matrix_connection* con, matrix_stack* inbox);

bool matrix_proto_get_client(matrix_connection* con, matrix_fd file, matrix_msg_id id);
bool matrix_proto_get_server(matrix_connection* con, matrix_fd filetext);

int8_t matrix_proto_del_client(matrix_connection* con, matrix_msg_id id);
bool matrix_proto_del_server(matrix_connection* con, int8_t retcode);