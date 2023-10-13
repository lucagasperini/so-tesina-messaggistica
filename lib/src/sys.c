#include "sys.h"

#include "dir.h"
#include "log.h"
#include "proto.h"

#include <string.h> // strncpy, strcat, strncat
#include <pthread.h> // pthread_mutex_lock, pthread_mutex_unlock

static char s_matrix_main_dir[MATRIX_PATH_LEN];

static matrix_fd s_matrix_server_auth_fd;
static matrix_fd s_matrix_server_id_fd;
static pthread_mutex_t s_matrix_id_mutex;

char* matrix_sys_home()
{
        char* home = getenv("HOME");
        if(home == NULL) {
                MATRIX_LOG_ERR("Cannot find home directory");
                return NULL;
        }

        return home;
}

static int s_matrix_main_dir_build(const char* main_dir)
{
        strncpy(s_matrix_main_dir, main_dir, MATRIX_PATH_LEN);

        return matrix_dir_create(s_matrix_main_dir);
}

static int s_matrix_server_auth_build()
{
        static char path[MATRIX_PATH_LEN];
        strncpy(path, s_matrix_main_dir, MATRIX_PATH_LEN);

        strcat(path, MATRIX_SYS_SEPARATOR MATRIX_SERVER_AUTH_NAME);

        s_matrix_server_auth_fd = matrix_file_open(path);
        
        // TODO: Add a real return
        return 0;
}

static int s_matrix_server_id_build()
{
        static char path[MATRIX_PATH_LEN];
        strncpy(path, s_matrix_main_dir, MATRIX_PATH_LEN);

        strcat(path, MATRIX_SYS_SEPARATOR MATRIX_SERVER_ID_NAME);

        s_matrix_server_id_fd = matrix_file_open(path);

        if(matrix_file_is_empty(s_matrix_server_id_fd)) {
                matrix_write_u32(s_matrix_server_id_fd, 0);
                matrix_file_reset_sync(s_matrix_server_id_fd);
        }

        matrix_msg_id id = matrix_sys_server_id();
        MATRIX_LOG_INFO("Starting with id %u", id);

        
        // TODO: Add a real return
        return 0;
}


char* matrix_sys_main_dir()
{
        return s_matrix_main_dir;
}

matrix_fd matrix_sys_server_auth()
{
        matrix_file_reset(s_matrix_server_auth_fd);
        return s_matrix_server_auth_fd;
}


int matrix_sys_server_build(const char* main_dir)
{
        s_matrix_main_dir_build(main_dir);
        s_matrix_server_auth_build();
        s_matrix_server_id_build();

        // TODO: Add a real return
        return 0;
}

int matrix_sys_client_build(const char* main_dir)
{
        s_matrix_main_dir_build(main_dir);
        // TODO: Add a real return
        return 0;
}

bool matrix_sys_user_dir(const char* username, char* buf)
{
        // TODO: CHECK BUFFER SIZE!!!
        strncpy(buf, s_matrix_main_dir, MATRIX_PATH_LEN);
        strcat(buf, MATRIX_SYS_SEPARATOR);
        strncat(buf, username, MATRIX_USERNAME_MAX_LEN);

        return true;
}

bool matrix_sys_user_dir_draft(const char* username, char* buf)
{
        // TODO: CHECK BUFFER SIZE!!!
        matrix_sys_user_dir(username, buf);
        strcat(buf, MATRIX_SYS_SEPARATOR MATRIX_SYS_USER_DRAFT);
}

bool matrix_sys_user_dir_inbox(const char* username, char* buf)
{
        // TODO: CHECK BUFFER SIZE!!!
        matrix_sys_user_dir(username, buf);
        strcat(buf, MATRIX_SYS_SEPARATOR MATRIX_SYS_USER_INBOX);
}

bool matrix_sys_user_dir_send(const char* username, char* buf)
{
        // TODO: CHECK BUFFER SIZE!!!
        matrix_sys_user_dir(username, buf);
        strcat(buf, MATRIX_SYS_SEPARATOR MATRIX_SYS_USER_SEND);
}

bool matrix_sys_user_dir_create(const char* username)
{
        char user_dir[MATRIX_PATH_LEN];
        matrix_sys_user_dir(username, user_dir);

        matrix_dir_create(user_dir);

        char user_section_dir[MATRIX_PATH_LEN];

        strncpy(user_section_dir, user_dir, MATRIX_PATH_LEN);
        strcat(user_section_dir, MATRIX_SYS_SEPARATOR MATRIX_SYS_USER_DRAFT);
        
        matrix_dir_create(user_section_dir);

        strncpy(user_section_dir, user_dir, MATRIX_PATH_LEN);
        strcat(user_section_dir, MATRIX_SYS_SEPARATOR MATRIX_SYS_USER_INBOX);

        matrix_dir_create(user_section_dir);

        strncpy(user_section_dir, user_dir, MATRIX_PATH_LEN);
        strcat(user_section_dir, MATRIX_SYS_SEPARATOR MATRIX_SYS_USER_SEND);

        matrix_dir_create(user_section_dir);

        return true;
}

matrix_msg_id matrix_sys_server_id()
{
        matrix_msg_id current_id;

        // reset file session to start
        matrix_file_reset(s_matrix_server_id_fd);

        // read first 4 byte where id is stored
        matrix_read_u32(s_matrix_server_id_fd, &current_id);

        // reset file session to start
        matrix_file_reset(s_matrix_server_id_fd);

        // return the id
        return current_id;
}

matrix_msg_id matrix_sys_server_id_add()
{
        // lock the id mutex
        pthread_mutex_lock(&s_matrix_id_mutex);
        // get current id on file
        matrix_msg_id current_id = matrix_sys_server_id();

        // check if limit reach
        if(current_id >= UINT32_MAX) {
                MATRIX_LOG_ERR("Current id is too big!");
                return 0;
        }
        matrix_msg_id next_id = current_id + 1;

        MATRIX_LOG_INFO("Got matrix message id %u", next_id);
        // write new value on id file
        matrix_write_u32(s_matrix_server_id_fd, next_id);
        // reset file session to start
        matrix_file_reset(s_matrix_server_id_fd);

        //unlock the id mutex
        pthread_mutex_unlock(&s_matrix_id_mutex);
        // return new id
        return next_id;
}