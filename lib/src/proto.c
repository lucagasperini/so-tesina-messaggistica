#include "proto.h"
#include "log.h"
#include "mem.h"
#include "msg.h"
#include "darray.h"

#include <arpa/inet.h> // inet_ntop, ntohs
#include <string.h> // strlen, strncpy memset

#define MATRIX_PROTO_HEADER_SIZE sizeof(header_magic_t) + sizeof(header_op_t) + sizeof(header_size_t)

bool matrix_proto_create_header(matrix_proto_header* header, header_op_t op, header_size_t sz)
{
        if(op > MATRIX_PROTO_OP_MAX) {
                MATRIX_LOG_ERR("Creation of TCP header with invalid operation [magic: %hhu, sz: %llu]", header->op, header->sz);
                return false;
        }

        header->magic = MATRIX_PROTO_MAGIC;
        header->op = op;
        header->sz = sz;
        return true;
}

bool matrix_proto_send_header(matrix_connection* con, matrix_proto_header* header)
{
        if(header->magic != MATRIX_PROTO_MAGIC) {
                MATRIX_LOG_ERR("TCP Send invalid magic [magic: %u]", header->magic);
                return false;
        }
        if(header->op > MATRIX_PROTO_OP_MAX) {
                MATRIX_LOG_ERR("TCP Send invalid operation [op: %hhu]", header->op);
                return false;
        }

        // TODO: Check if payload is NULL but header->sz is not 0, 
        // or payload NOT is NULL but but header->sz is 0

        size_t i = 0;
        char pkg[MATRIX_PROTO_HEADER_SIZE];

        matrix_pack_u32(pkg, &i, header->magic);
        matrix_pack_u8(pkg, &i, header->op);
        matrix_pack_u64(pkg, &i, header->sz);

        // MATRIX_PROTO_HEADER_SIZE == i
        if(matrix_send(con, pkg, MATRIX_PROTO_HEADER_SIZE) == -1) {
                return false;
        }

        MATRIX_LOG_NET_TO(con, "op=%s sz=%llu", MATRIX_PROTO_OP_STRING(header->op), header->sz);

        return true;
}

void matrix_proto_send_payload(matrix_connection* con, const char* payload, size_t sz)
{
        size_t tot = 0;

        while(sz - tot > MATRIX_BLOCK) {
                matrix_send_blk(con, payload + tot);

                tot += MATRIX_BLOCK;
        }

        if(sz - tot > 0) {
                matrix_send(con, payload + tot, sz - tot);
        }

}

bool matrix_proto_receive_header(matrix_connection* con, matrix_proto_header* header)
{
        size_t i = 0;
        char pkg[MATRIX_PROTO_HEADER_SIZE];

        if(matrix_receive(con, pkg, MATRIX_PROTO_HEADER_SIZE) == -1) {
                return false;
        }

        matrix_unpack_u32(pkg, &i, &header->magic);
        matrix_unpack_u8(pkg, &i, &header->op);
        matrix_unpack_u64(pkg, &i, &header->sz);

        if(header->magic != MATRIX_PROTO_MAGIC) {
                MATRIX_LOG_ERR("TCP Receive invalid magic [magic: %u]", header->magic);
                return false;
        }

        if(header->op > MATRIX_PROTO_OP_MAX) {
                MATRIX_LOG_ERR("TCP Receive invalid operation [magic: %hhu]", header->op);
                return false;
        }

        MATRIX_LOG_NET_FROM(con, "op=%s sz=%llu", MATRIX_PROTO_OP_STRING(header->op), header->sz);

        return true;
}

void matrix_proto_receive_payload(matrix_connection* con, char* payload, size_t sz)
{
        if(sz == 0) {
                MATRIX_LOG_INFO("Got a payload without size");
                return;
        }
        size_t tot = 0;
        // buffer block
        char pkg[MATRIX_BLOCK];

        // get all blocks and store each of them on the payload
        while(sz - tot > MATRIX_BLOCK) {
                matrix_receive_blk(con, pkg);
                matrix_pack(payload, &tot, pkg, MATRIX_BLOCK);
        }
        // get last block and store it on the payload
        if(sz - tot > 0) {
                matrix_receive(con, pkg, sz - tot);
                matrix_pack(payload, &tot, pkg, sz - tot);
        }
}

bool matrix_proto_handshake_client(matrix_connection* con)
{
        matrix_proto_header reset_client, reset_server;
        matrix_proto_create_header(&reset_client, MATRIX_PROTO_RESET_CLIENT, 0);
        if(!matrix_proto_send_header(con, &reset_client)) {
                return false;
        }
        if(!matrix_proto_receive_header(con, &reset_server)) {
                return false;
        }

        if(reset_server.op != MATRIX_PROTO_RESET_SERVER) {
                MATRIX_LOG_ERR("Handshake not expecting operation [op: %s]", 
                        MATRIX_PROTO_OP_STRING(reset_server.op));
                return false;
        }

        return true;
}

bool matrix_proto_handshake_server(matrix_connection* con)
{
        matrix_proto_header reset_client, reset_server;
        matrix_proto_create_header(&reset_server, MATRIX_PROTO_RESET_SERVER, 0);
        if(!matrix_proto_send_header(con, &reset_server)) {
                return false;
        }
        if(!matrix_proto_receive_header(con, &reset_client)) {
                return false;
        }

        if(reset_client.op != MATRIX_PROTO_RESET_CLIENT) {
                MATRIX_LOG_ERR("Handshake not expecting operation [op: %s]", 
                        MATRIX_PROTO_OP_STRING(reset_client.op));
                return false;
        }

        return true;

}

bool matrix_proto_auth_client(matrix_connection* con, const char* username, const char* userpass)
{
        matrix_proto_header control, ack;

        size_t username_len = strlen(username);
        size_t userpass_len = strlen(userpass);

        size_t payload_size = username_len + userpass_len + 2;
        char payload[payload_size];

        
        if(username_len >= MATRIX_USERNAME_MAX_LEN) {
                MATRIX_LOG_ERR("Username too long");
                return false;
        }

        
        if(userpass_len >= MATRIX_USERPASS_MAX_LEN) {
                MATRIX_LOG_ERR("Userpass too long");
                return false;
        }

        // TODO: Pack this stuff
        memset(payload, 0, payload_size);

        strncpy(payload, username, username_len);
        strncpy(payload + username_len + 1, userpass, userpass_len);

        matrix_proto_create_header(&control, MATRIX_PROTO_CONTROL, payload_size);

        if(!matrix_proto_send_header(con, &control)) {
                return false;
        }

        matrix_proto_send_payload(con, payload, payload_size);
        
        if(!matrix_proto_receive_header(con, &ack)) {
                return false;
        }

        if(ack.op != MATRIX_PROTO_ACK) {
                MATRIX_LOG_ERR("Auth not expecting operation [op: %s]", 
                        MATRIX_PROTO_OP_STRING(ack.op));
                return false;
        }

        char* res_payload;
        MATRIX_MALLOC(res_payload, ack.sz);

        matrix_proto_receive_payload(con, res_payload, ack.sz);

        uint8_t res_code = res_payload[0];
        
        if(res_code != MATRIX_PROTO_CODE_AUTH_OK) {
                MATRIX_FREE(res_payload, "Protocol Payload ACK");
                return false;
        }

        MATRIX_FREE(res_payload, "Protocol Payload ACK");
        return true;
}

bool matrix_proto_auth_request_server(matrix_connection* con, char* username, char* userpass)
{
        matrix_proto_header control;

        if(!matrix_proto_receive_header(con, &control)) {
                return false;
        }

        if(control.op != MATRIX_PROTO_CONTROL) {
                MATRIX_LOG_ERR("Auth not expecting operation [op: %s]", 
                        MATRIX_PROTO_OP_STRING(control.op));
                return false;
        }

        char* payload;
        MATRIX_MALLOC(payload, control.sz);

        matrix_proto_receive_payload(con, payload, control.sz);

        // TODO: Unpack this
        size_t username_len = strlen(payload);
        if(username_len >= MATRIX_USERNAME_MAX_LEN) {
                MATRIX_LOG_ERR("Username too long");
                goto ret_false;
        }

        size_t userpass_len = strlen(payload + username_len + 1);
        if(userpass_len >= MATRIX_USERPASS_MAX_LEN) {
                MATRIX_LOG_ERR("Userpass too long");
                goto ret_false;
        }

        strncpy(username, payload, username_len + 1);
        strncpy(userpass, payload + username_len + 1, userpass_len + 1);

        MATRIX_FREE(payload, "Protocol Payload CONTROL");
        return true;

ret_false:
        MATRIX_FREE(payload, "Protocol Payload CONTROL");
        return false;
}

bool matrix_proto_auth_response_server(matrix_connection* con, uint8_t code)
{
        matrix_proto_header ack;

        matrix_proto_create_header(&ack, MATRIX_PROTO_ACK, sizeof(code));

        if(!matrix_proto_send_header(con, &ack)) {
                return false;
        }

        matrix_proto_send_payload(con, &code, sizeof(code));
        return true;
}

int8_t matrix_proto_put_client(matrix_connection* con, const char* dest, const char* subject, matrix_fd filetext)
{
        matrix_proto_header put;

        size_t textlen = matrix_file_size(filetext);
        
        char payload_head[MATRIX_USERNAME_MAX_LEN + MATRIX_SUBJECT_MAX_LEN + sizeof(uint64_t) + (sizeof(uint32_t) * 2)];
        size_t i = 0;
        matrix_pack_str(payload_head, &i, dest);
        matrix_pack_str(payload_head, &i, subject);
        matrix_pack_u32(payload_head, &i, textlen);
        MATRIX_LOG_DEBUG("Sending payload for message [dest: \"%s\", subject: \"%s\", textlen: %llu]", dest, subject, textlen);

        if(!matrix_proto_create_header(&put, MATRIX_PROTO_CLIENT_PUT, i + textlen)) {
                MATRIX_LOG_ERR("Client cannot PUT, payload is too big [dest: \"%s\", subject: \"%s\", payload_len: %llu]",
                        dest, subject, i + textlen);
                return -1;
        }

        if(!matrix_proto_send_header(con, &put)) {
                return -1;
        }

        matrix_proto_send_payload(con, payload_head, i);

        char payload_block[MATRIX_BLOCK];
        while(i = matrix_read_blk(filetext, payload_block)) {
                matrix_proto_send_payload(con, payload_block, i);
        }

        matrix_proto_header res_header;


        if(!matrix_proto_receive_header(con, &res_header)) {
                return false;
        }

        if(res_header.op != MATRIX_PROTO_SERVER_PUT) {
                MATRIX_LOG_ERR("PUT not expecting operation [op: %s]", 
                        MATRIX_PROTO_OP_STRING(res_header.op));
                return false;
        }

        char* res_payload;
        MATRIX_MALLOC(res_payload, res_header.sz);

        matrix_proto_receive_payload(con, res_payload, res_header.sz);

        int8_t retcode = 0;
        matrix_unpack_i8(res_payload, NULL, &retcode);

        MATRIX_FREE(res_payload, "Protocol Payload PUT");
        return retcode;

}

bool matrix_proto_put_server(matrix_connection* con, uint8_t code)
{
        matrix_proto_header ack;

        matrix_proto_create_header(&ack, MATRIX_PROTO_SERVER_PUT, sizeof(uint8_t));

        if(!matrix_proto_send_header(con, &ack)) {
                return false;
        }

        matrix_proto_send_payload(con, &code, sizeof(uint8_t));

        return true;
}

matrix_stack_str* matrix_proto_users_client(matrix_connection* con)
{
        matrix_proto_header send, receive;

        matrix_proto_create_header(&send, MATRIX_PROTO_CLIENT_USERS,0);
        if(!matrix_proto_send_header(con, &send)) {
                return false;
        }

        if(!matrix_proto_receive_header(con, &receive)) {
                return false;
        }

        if(receive.op != MATRIX_PROTO_SERVER_USERS) {
                MATRIX_LOG_ERR("USERS not expecting operation [op: %s]", 
                        MATRIX_PROTO_OP_STRING(receive.op));
                return false;
        }


        char* payload;
        MATRIX_MALLOC(payload, receive.sz);

        matrix_proto_receive_payload(con, payload, receive.sz);

        matrix_stack_str* head = NULL;
        size_t i = 0;
        char buffer[MATRIX_USERNAME_MAX_LEN];
        while(i < receive.sz) {
                if(!matrix_unpack_buf(payload, &i, buffer, MATRIX_USERNAME_MAX_LEN)) {
                        goto err;
                }
                head = matrix_stack_str_push(head, buffer);
        }

        MATRIX_FREE(payload, "Protocol Payload USERS");
        return head;
err:
        MATRIX_FREE(payload, "Protocol Payload USERS");
        return NULL;
}

bool matrix_proto_users_server(matrix_connection* con, matrix_stack_str* userlist, size_t num_users)
{
        matrix_proto_header send;
        size_t i = 0;
        char* buffer;
        MATRIX_MALLOC(buffer, num_users * MATRIX_USERNAME_MAX_LEN);
        matrix_stack_str* tmp = userlist;
        while(tmp) {
                matrix_pack_str(buffer, &i, tmp->data);
                tmp = tmp->next;
        }

        matrix_proto_create_header(&send, MATRIX_PROTO_SERVER_USERS, i);
        if(!matrix_proto_send_header(con, &send)) {
                goto err;
        }

        matrix_proto_send_payload(con, buffer, i);


        MATRIX_FREE(buffer);
        return true;
err:
        MATRIX_FREE(buffer);
        return false;
}

matrix_stack* matrix_proto_inbox_client(matrix_connection* con)
{
        matrix_proto_header send, receive;

        matrix_proto_create_header(&send, MATRIX_PROTO_CLIENT_INBOX,0);
        if(!matrix_proto_send_header(con, &send)) {
                return false;
        }

        if(!matrix_proto_receive_header(con, &receive)) {
                return false;
        }

        if(receive.op != MATRIX_PROTO_SERVER_INBOX) {
                MATRIX_LOG_ERR("INBOX not expecting operation [op: %s]", 
                        MATRIX_PROTO_OP_STRING(receive.op));
                return false;
        }

        // Check if there is no new message
        if(receive.sz == 0) {
                return NULL;
        }

        char* payload;
        MATRIX_MALLOC(payload, receive.sz);

        matrix_proto_receive_payload(con, payload, receive.sz);

        size_t num_elements = receive.sz / MATRIX_PAYLOAD_LIST_OBJ;
        matrix_msg_info buffer;
        matrix_stack* head = NULL;

        for(size_t block_index = 0; block_index < num_elements; block_index++) {
                size_t i = 0;
                size_t block_offset = block_index * MATRIX_PAYLOAD_LIST_OBJ;
                matrix_unpack_u32(payload + block_offset, &i, &buffer.id);
                matrix_unpack_str(payload + block_offset, &i, &buffer.sender);
                matrix_unpack_str(payload + block_offset, &i, &buffer.destination);
                matrix_unpack_str(payload + block_offset, &i, &buffer.subject);
                head = matrix_stack_push(head, &buffer, sizeof(matrix_msg_info));
        }

        MATRIX_FREE(payload, "Protocol Payload INBOX");
        return head;
err:
        MATRIX_FREE(payload, "Protocol Payload INBOX");
        return NULL;
}

bool matrix_proto_inbox_server(matrix_connection* con, matrix_stack* inbox)
{
        matrix_proto_header send;
        size_t i = 0; 
        matrix_stack* tmp = inbox;
        matrix_msg_info* ptr_info;

        // create a dynamic array to store message list
        matrix_darray da;
        matrix_darray_create(
                &da, 
                MATRIX_PAYLOAD_LIST_OBJ,
                10
        );

        char buffer[MATRIX_PAYLOAD_LIST_OBJ];
        while(tmp) {
                i = 0;
                ptr_info = (matrix_msg_info*)tmp->entry.data;
                matrix_pack_u32(buffer, &i, ptr_info->id);
                matrix_pack_str(buffer, &i, ptr_info->sender);
                matrix_pack_str(buffer, &i, ptr_info->destination);
                matrix_pack_str(buffer, &i, ptr_info->subject);
                
                matrix_darray_push(&da, buffer);
                tmp = tmp->next;
        }

        size_t used_sz = matrix_darray_used_sz(&da);

        matrix_proto_create_header(&send, MATRIX_PROTO_SERVER_INBOX, used_sz);
        if(!matrix_proto_send_header(con, &send)) {
                goto err;
        }

        void* payload = matrix_darray_ptr(&da);

        matrix_proto_send_payload(con, payload, used_sz);

        matrix_darray_free(&da);
        return true;

err:
        matrix_darray_free(&da);
        return false;
}

bool matrix_proto_get_client(matrix_connection* con, matrix_fd file, matrix_msg_id id)
{
        matrix_proto_header send, receive;

        matrix_proto_create_header(&send, MATRIX_PROTO_CLIENT_GET, sizeof(matrix_msg_id));
        char id_payload[sizeof(matrix_msg_id)];
        matrix_pack_u32(id_payload, NULL, id);

        if(!matrix_proto_send_header(con, &send)) {
                return false;
        }

        matrix_proto_send_payload(con, id_payload, sizeof(matrix_msg_id));

        if(!matrix_proto_receive_header(con, &receive)) {
                return false;
        }

        if(receive.op != MATRIX_PROTO_SERVER_GET) {
                MATRIX_LOG_ERR("GET not expecting operation [op: %s]", 
                        MATRIX_PROTO_OP_STRING(receive.op));
                return false;
        }
        size_t tot = 0;
        char buffer[MATRIX_BLOCK];
        while(receive.sz - tot > MATRIX_BLOCK) {
                matrix_proto_receive_payload(con, buffer, MATRIX_BLOCK);
                matrix_write_blk(file, buffer);
                tot += MATRIX_BLOCK;
        }

        if(receive.sz - tot > 0) {
                matrix_proto_receive_payload(con, buffer, receive.sz - tot);
                matrix_write(file, buffer, receive.sz - tot);
        }

        return true;
}

bool matrix_proto_get_server(matrix_connection* con, matrix_fd filetext)
{
        matrix_proto_header send, receive;
        size_t filesize;

        // if there is no file, just send header without payload, else payload size is filetext size
        if(filetext == -1) {
                filesize = 0;
        } else {
                filesize = matrix_file_size(filetext);
        }

        matrix_proto_create_header(&send, MATRIX_PROTO_SERVER_GET, filesize);

        if(!matrix_proto_send_header(con, &send)) {
                return false;
        }

        if(filesize == 0) {
                return false;
        }

        char buffer[MATRIX_BLOCK];
        size_t i = 0;
        while(i = matrix_read_blk(filetext, buffer)) {
                matrix_proto_send_payload(con, buffer, i);
        }

        return true;
}

int8_t matrix_proto_del_client(matrix_connection* con, matrix_msg_id id)
{
        matrix_proto_header send, receive;

        char buf[sizeof(matrix_msg_id)];
        matrix_pack_u32(buf, NULL, id);
        matrix_proto_create_header(&send, MATRIX_PROTO_CLIENT_DEL, sizeof(matrix_msg_id));

        if(!matrix_proto_send_header(con, &send)) {
                return false;
        }

        matrix_proto_send_payload(con, buf, sizeof(matrix_msg_id));

        if(!matrix_proto_receive_header(con, &receive)) {
                return false;
        }

        if(receive.op != MATRIX_PROTO_SERVER_DEL) {
                MATRIX_LOG_ERR("DEL not expecting operation [op: %s]", 
                        MATRIX_PROTO_OP_STRING(receive.op));
                return false;
        }

        char* payload;
        MATRIX_MALLOC(payload, receive.sz);
        matrix_proto_receive_payload(con, payload, receive.sz);

        int8_t retcode;
        matrix_unpack_i8(payload, NULL, &retcode); 
        MATRIX_FREE(payload);
        return retcode;
        
err:
        MATRIX_FREE(payload);
        return -1;
}

bool matrix_proto_del_server(matrix_connection* con, int8_t retcode)
{
        matrix_proto_header send;

        char buf[sizeof(int8_t)];
        matrix_pack_i8(buf, NULL, retcode);
        matrix_proto_create_header(&send, MATRIX_PROTO_SERVER_DEL, sizeof(int8_t));

        if(!matrix_proto_send_header(con, &send)) {
                return false;
        }

        matrix_proto_send_payload(con, buf, sizeof(int8_t));

        return true;
}