#include "msg.h"
#include "log.h"
#include "proto.h"
#include "sys.h"
#include "utils.h"

#include <string.h> // strncpy, strcat
#include <fcntl.h>
#include <stdio.h> // sprintf

#define MATRIX_MSG_LABEL_FROM "From: "
#define MATRIX_MSG_LABEL_TO "To: "
#define MATRIX_MSG_LABEL_SUBJECT "Subject: "

#define MATRIX_MSG_LINE_MAX 1024
#define MATRIX_MSG_ID_STR_LEN 16

#define MATRIX_MSG_INFO_NUMROW 3

matrix_fd matrix_msg_create_draft(const char* sender, const char* dest, const char* subject)
{
        char draft_path[MATRIX_PATH_LEN];

        matrix_sys_user_dir_draft(sender, draft_path);

        char file_path[MATRIX_PATH_LEN];

        sprintf(file_path, "%s" MATRIX_SYS_SEPARATOR "%s.eml", draft_path, dest);

        matrix_fd fd = matrix_file_open_new(file_path);

        return fd;
}

matrix_fd matrix_msg_create_file(const char* dir, const char* sender, const char* dest, const char* subject, matrix_msg_id* id)
{
        char file_path[MATRIX_PATH_LEN];

        *id = matrix_sys_server_id_add();
        sprintf(file_path, "%s" MATRIX_SYS_SEPARATOR "%u.eml", dir, *id);

        matrix_fd fd = matrix_file_open(file_path);

        matrix_msg_append_field(fd, MATRIX_MSG_LABEL_FROM, sender);
        matrix_msg_append_field(fd, MATRIX_MSG_LABEL_TO, dest);
        matrix_msg_append_field(fd, MATRIX_MSG_LABEL_SUBJECT, subject);

        return fd;
}

bool matrix_msg_create(const char* sender, const char* dest, const char* subject, const char* text)
{
        char path[MATRIX_PATH_LEN];

        matrix_sys_user_dir_send(sender, path);
        matrix_msg_id id;
        matrix_fd fd = matrix_msg_create_file(path, sender, dest, subject, &id);
        // TODO: What about VERY long text? seems need a rework for handle this
        if(text != NULL) {
                matrix_write_string(fd, text);
        }
        matrix_sys_user_dir_inbox(dest, path);
        char dest_path[MATRIX_PATH_LEN];
        sprintf(dest_path, "%s" MATRIX_SYS_SEPARATOR "%u.eml", path, id);

        matrix_file_copy_fd(fd, dest_path);
        // TODO: Add return false
        return true;
}

int matrix_msg_append(matrix_fd fd, const char* text)
{
        // TODO: Check if fails?
        matrix_write_string(fd, text);
        return 0;
}

int matrix_msg_append_field(matrix_fd fd, const char* label, const char* value)
{
        // TODO: Check if fails?
        matrix_write_string(fd, label);
        matrix_write_string(fd, value);
        matrix_write_char(fd, '\n');
        return 0;
}

matrix_fd matrix_msg_receive_file(matrix_connection* con, matrix_msg_id id, const char* username)
{
        char file[MATRIX_PATH_LEN];
        char inbox[MATRIX_PATH_LEN];
        // Get user inbox directory
        matrix_sys_user_dir_inbox(username, inbox);
        // Get file path where write on client directory
        sprintf(file, "%s" MATRIX_SYS_SEPARATOR "%u.eml", inbox, id);
        // Create this file
        matrix_fd fd = matrix_file_open(file);

        MATRIX_ASSERT(fd != -1)

        // Try to get this file from server
        if(!matrix_proto_get_client(con, fd, id)) {
                MATRIX_LOG_ERR("Cannot fetch message [file: %s]", file);
                // Close this file and delete from client inbox
                matrix_file_close(fd);
                matrix_file_delete(file);
                return -1;
        }

        matrix_file_reset_sync(fd);

        return fd;
}

bool matrix_msg_send_file(matrix_connection* con, matrix_msg_id id, const char* username)
{
        char file[MATRIX_PATH_LEN];
        char inbox[MATRIX_PATH_LEN];
        matrix_sys_user_dir_inbox(username, inbox);
        sprintf(file, "%s" MATRIX_SYS_SEPARATOR "%u.eml", inbox, id);
        matrix_fd fd = matrix_file_open_ro(file);

        if(!matrix_proto_get_server(con, fd)) {
                return false;
        }

        matrix_file_close(fd);

        return true;
}

bool matrix_msg_delete_file(matrix_msg_id id, const char* username)
{
        char file[MATRIX_PATH_LEN];
        char inbox[MATRIX_PATH_LEN];
        matrix_sys_user_dir_inbox(username, inbox);
        sprintf(file, "%s" MATRIX_SYS_SEPARATOR "%u.eml", inbox, id);
        return matrix_file_delete(file);
}


bool matrix_msg_info_file(const char* filepath, matrix_msg_info* info)
{
        matrix_fd file = matrix_file_open(filepath);
        char line[MATRIX_MSG_LINE_MAX];
        size_t nbytes = 0;

        char id_str[MATRIX_MSG_ID_STR_LEN];
        matrix_file_parse_name(filepath, id_str);

        long id;
        if(!matrix_strtol(id_str, &id)) {
                MATRIX_LOG_ERR("Cannot convert string to matrix_msg_id [str: %s]", id_str);
                return false;
        }

        info->id = id;

        for(int i = 0; i < MATRIX_MSG_INFO_NUMROW; i++) {
                nbytes = matrix_read_line(file, line, MATRIX_MSG_LINE_MAX);
                if(nbytes <= 0) {
                        MATRIX_LOG_ERR("Email file line cannot be len 0 [file: %s]", filepath);
                        return false;
                }

                if(strncmp(line, MATRIX_MSG_LABEL_FROM, strlen(MATRIX_MSG_LABEL_FROM)) == 0) {
                        MATRIX_CALLOC(info->sender, nbytes - strlen(MATRIX_MSG_LABEL_FROM), 1);
                        strncpy(
                                info->sender, 
                                line + strlen(MATRIX_MSG_LABEL_FROM), 
                                nbytes - strlen(MATRIX_MSG_LABEL_FROM) - 1 // removing new line
                        );

                } else if(strncmp(line, MATRIX_MSG_LABEL_TO, strlen(MATRIX_MSG_LABEL_TO)) == 0) {
                        MATRIX_CALLOC(info->destination, nbytes - strlen(MATRIX_MSG_LABEL_TO) , 1);
                        strncpy(
                                info->destination, 
                                line + strlen(MATRIX_MSG_LABEL_TO), 
                                nbytes - strlen(MATRIX_MSG_LABEL_TO) - 1 // removing new line
                        );
                } else if(strncmp(line, MATRIX_MSG_LABEL_SUBJECT, strlen(MATRIX_MSG_LABEL_SUBJECT)) == 0) {
                        MATRIX_CALLOC(info->subject, nbytes - strlen(MATRIX_MSG_LABEL_SUBJECT), 1);
                        strncpy(
                                info->subject, 
                                line + strlen(MATRIX_MSG_LABEL_SUBJECT), 
                                nbytes - strlen(MATRIX_MSG_LABEL_SUBJECT) - 1 // removing new line
                        );
                } else {
                        MATRIX_LOG_ERR("Malformed email file [file: %s]", filepath);
                        return false;
                }
        }


        return true;
}

bool matrix_msg_info_inbox(const char* user, const char* file, matrix_msg_info* info)
{
        char inbox_dir[MATRIX_PATH_LEN];
        matrix_sys_user_dir_inbox(user, inbox_dir);
        strcat(inbox_dir, MATRIX_SYS_SEPARATOR);
        strcat(inbox_dir, file);
        return matrix_msg_info_file(inbox_dir, info);
}