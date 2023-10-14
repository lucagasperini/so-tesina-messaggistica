#include "users.h"
#include "file.h"
#include "proto.h"
#include "log.h"
#include "sys.h"

#include "dir.h"

#include <string.h> // strtok_r, strncmp, strncpy, strncat, strcat, strlen


bool matrix_users_add(const char* username, const char* userpass)
{
        size_t len = MATRIX_USERNAME_MAX_LEN + MATRIX_USERPASS_MAX_LEN + 2;
        char buffer[len];

        matrix_fd auth_fd = matrix_sys_server_auth();

        while(matrix_read_line(auth_fd, buffer, len)) {
                char* user = strtok(buffer, ":");
                if(strncmp(username, user, MATRIX_USERNAME_MAX_LEN) == 0) {
                        MATRIX_LOG_ERR("User already exists: %s", username);
                        return false;
                }
        }

        size_t username_len = strlen(username);
        size_t userpass_len = strlen(userpass);

        strncpy(buffer, username, username_len + 1);
        strcat(buffer, ":");
        strncat(buffer, userpass, userpass_len + 1);

        matrix_write_line(auth_fd, buffer);

        //TODO: Maybe check if user dir is created when try to login
        matrix_sys_user_dir_create(username);

        MATRIX_LOG_INFO("User added: %s", username);

        return true;
}

bool matrix_users_find(const char* username)
{
        size_t len = MATRIX_USERNAME_MAX_LEN + MATRIX_USERPASS_MAX_LEN + 2;
        char buffer[len];
        char* save_ptr;

        matrix_fd auth_fd = matrix_sys_server_auth();
        
        while(matrix_read_line(auth_fd, buffer, len)) {
                char* user = strtok_r(buffer, ":", &save_ptr);
                if(strncmp(username, user, MATRIX_USERNAME_MAX_LEN) == 0) {
                        MATRIX_LOG_INFO("User found: %s", username);
                        return true;
                }
        }

        MATRIX_LOG_INFO("User not found: %s", username);
        return false;
}

bool matrix_users_login(const char* username, const char* userpass)
{
        size_t len = MATRIX_USERNAME_MAX_LEN + MATRIX_USERPASS_MAX_LEN + 2;
        char buffer[len];
        char* save_ptr;

        matrix_fd auth_fd = matrix_sys_server_auth();
        
        while(matrix_read_line(auth_fd, buffer, len)) {
                char* user = strtok_r(buffer, ":", &save_ptr);
                if(strncmp(username, user, MATRIX_USERNAME_MAX_LEN) == 0) {
                        char* pass = strtok_r(NULL, "\n", &save_ptr);
                        if(strncmp(userpass, pass, MATRIX_USERPASS_MAX_LEN) == 0) {
                                MATRIX_LOG_INFO("User login: %s", username);
                                return true;
                        } else {
                                MATRIX_LOG_INFO("User login failed: %s", username);
                                return false;
                        }
                }
        }

        MATRIX_LOG_INFO("User not found: %s", username);
        return false;

}

matrix_stack_str* matrix_users_list(size_t* num)
{
        char line[MATRIX_USERNAME_MAX_LEN];
        char* save_ptr;
        size_t i = 0;

        matrix_stack_str* head = NULL;

        matrix_fd auth_fd = matrix_sys_server_auth();

        while(matrix_read_line(auth_fd, line, MATRIX_USERNAME_MAX_LEN)) {
                char* user = strtok_r(line, ":", &save_ptr);
                head = matrix_stack_str_push(head, user);
                i++;
        }
        
        if(num != NULL) {
                *num = i;
        }

        return head;
}

matrix_stack* matrix_user_inbox(const char* user)
{
        char inbox_dir[MATRIX_PATH_LEN];
        matrix_sys_user_dir_inbox(user, inbox_dir);

        matrix_dir dir = matrix_dir_open(inbox_dir);
        matrix_stack_str* dir_head = matrix_dir_entries_name(dir);
        matrix_stack_str* dir_tmp = dir_head;

        matrix_stack* info_head = NULL;
        matrix_msg_info buffer;

        while(dir_tmp != NULL) {
                matrix_msg_info_inbox(user, dir_tmp->data, &buffer);
                info_head = matrix_stack_push(info_head, &buffer, sizeof(matrix_msg_info));

                dir_tmp = dir_tmp->next;
        }

        matrix_stack_str_free(dir_head);

        return info_head;
}