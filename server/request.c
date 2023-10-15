#include "request.h"
#include "log.h"
#include "proto.h"
#include "msg.h"
#include "users.h"
#include "sys.h"
#include "mem.h"

#include <string.h> // strlen, memcpy

#define MATRIX_STRLIST_PARSE_MAX 64


bool request_start_server(client_thread_info* tinfo)
{
        if(!matrix_proto_handshake_server(&(tinfo->con))) {
                return false;
        }

        char username[MATRIX_USERNAME_MAX_LEN];
        char userpass[MATRIX_USERPASS_MAX_LEN];

        if(!matrix_proto_auth_request_server(&(tinfo->con), username, userpass)) {
                return false;
        }

        if(matrix_users_login(username, userpass)) {
                matrix_proto_auth_response_server(&(tinfo->con), MATRIX_PROTO_CODE_AUTH_OK);
                strncpy(tinfo->username, username, MATRIX_USERNAME_MAX_LEN);
                return true;
        } else {
                matrix_proto_auth_response_server(&(tinfo->con), MATRIX_PROTO_CODE_AUTH_FAIL);
                return false;
        }
}


matrix_request_result request_put(matrix_connection* con, const char* username, header_size_t payload_sz)
{
        matrix_request_result rescode = MATRIX_PROTO_CODE_OK;

        size_t i = 0;
        char* dest;
        char* subject;
        char* text;

        char* payload;
        MATRIX_MALLOC(payload, payload_sz);

        // get a payload with message informations
        matrix_proto_receive_payload(con, payload, payload_sz);

        matrix_unpack_str(payload, &i, &dest);
        matrix_unpack_str(payload, &i, &subject);
        // WARNING: This can be NULL
        matrix_unpack_str(payload, &i, &text);

        // check if message informations are correct
        if(strlen(dest) > MATRIX_USERPASS_MAX_LEN) {
                MATRIX_LOG_ERR("Protocol package with destination too big! [sz: %lu]", strlen(dest));
                rescode = MATRIX_PROTO_CODE_INVPAYLOAD;
                goto err;
        }

        if(strlen(subject) > MATRIX_SUBJECT_MAX_LEN) {
                MATRIX_LOG_ERR("Protocol package with subject too big! [sz: %lu]", strlen(subject));
                rescode = MATRIX_PROTO_CODE_INVPAYLOAD;
                goto err;
        }

        // check if user exists
        if(!matrix_users_find(dest)) {
                rescode = MATRIX_PROTO_CODE_NOTFOUND;
        } else {
                // if so, create a message into his mailbox
                if(!matrix_msg_create(username, dest, subject, text)) {
                        rescode = MATRIX_PROTO_CODE_INTERR;
                }
        }

err:

        if(rescode == MATRIX_PROTO_CODE_OK) {
                MATRIX_LOG_INFO("Server got a new email from %s to %s", username, dest);
        } else {
                MATRIX_LOG_ERR("Server got a new email with error %s from %s to %s", 
                        MATRIX_PROTO_CODE_STRING(rescode), 
                        username, 
                        dest
                );
        }

        MATRIX_FREE(payload, "Free payload");
        
        // TODO: Add error code
        matrix_proto_put_server(con, rescode);

        return rescode;
}

matrix_request_result request_users(matrix_connection* con)
{
        // TODO: Add define of this
        size_t num;

        matrix_stack_str* head = matrix_users_list(&num);

        if(!matrix_proto_users_server(con, head, num)) {
                return -1;
        }

        return 0;
}

matrix_request_result request_inbox(matrix_connection* con, const char* username)
{
        matrix_stack* info_stack = matrix_user_inbox(username);

        if(!matrix_proto_inbox_server(con, info_stack)) {
                matrix_stack_free(info_stack);
                return -1;
        }
        matrix_stack_free(info_stack);
        return 0;
}

matrix_request_result request_get(matrix_connection* con, const char* username)
{
        //get message id from payload
        matrix_msg_id id;
        matrix_proto_receive_payload(con, &id, sizeof(matrix_msg_id));

        // send to the client the message file stored on server
        if(!matrix_msg_send_file(con, id, username)) {
                // TODO: What about those return code????
                return 1;
        }

        return 0;
}

matrix_request_result request_del(matrix_connection* con, const char* username)
{
        int8_t retcode = 0;

        //get message id from payload
        matrix_msg_id id;
        matrix_proto_receive_payload(con, &id, sizeof(matrix_msg_id));

        // delete this message from the server
        if(!matrix_msg_delete_file(id, username)) {
                retcode = 1;
        }

        // send return code to client
        matrix_proto_del_server(con, retcode);
        return retcode;
}


void request_loop(client_thread_info* tinfo)
{
        matrix_proto_header header;

        // TODO: Do something with retcode!
        matrix_request_result retcode = 0;

        while(matrix_proto_receive_header(&(tinfo->con), &header) && !retcode) {
                if(header.op == MATRIX_PROTO_CLIENT_PUT) {
                        retcode = request_put(&(tinfo->con), tinfo->username, header.sz);
                } else if (header.op == MATRIX_PROTO_CLIENT_USERS) {
                        retcode = request_users(&(tinfo->con));
                } else if (header.op == MATRIX_PROTO_CLIENT_INBOX) {
                        retcode = request_inbox(&(tinfo->con), tinfo->username);
                } else if (header.op == MATRIX_PROTO_CLIENT_GET) {
                        retcode = request_get(&(tinfo->con), tinfo->username);
                } else if (header.op == MATRIX_PROTO_CLIENT_DEL) {
                        retcode = request_del(&(tinfo->con), tinfo->username);
                } else {
                        MATRIX_LOG_ERR("Undefined operation [op: %s]", MATRIX_PROTO_OP_STRING(header.op));
                }
        }
}