#include <matrix.h>

#include <stdio.h> // printf, fprintf, puts, fgets, fflush, stderr, stdin, stdout
#include <stdlib.h> // exit, strtol, EXIT_SUCCESS, EXIT_FAILURE, NULL
#include <stdint.h> // uint16_t
#include <errno.h> // errno
#include <string.h> // strtok, strlen, strchr, strncmp
#include <ctype.h>  // tolower
#include <signal.h> // signal


#define STR_HOST_DEFAULT "127.0.0.1"
#define INPUT_BUFFER_LEN 4096
#define LOG_FILE_PATH "matrix-client.log"

#define SEPARATOR_LINE "-------------------------------------------"

char input_buffer[INPUT_BUFFER_LEN];

uint16_t dest_port;
char dest_host[STR_HOST_LEN];

char login_username[MATRIX_USERNAME_MAX_LEN];

matrix_connection con_client, con_server;

int op_help()
{
        MATRIX_LOG_INFO("Help operation");

        puts("list/l - Show inbox messages\n"
             "send/s - Send a message\n"
             "read/r - Show a message\n"
             "delete/d - Delete a message\n"
             "users/u - Show user on server\n"
             "help/h - Show this help message\n"
             "quit/q - Exit from program");
        fflush(stdout);
        return 0;
}


int op_list()
{
        MATRIX_LOG_INFO("List operation");
        matrix_stack* head_inbox;
        head_inbox = matrix_proto_inbox_client(&con_server);
        matrix_stack* tmp = head_inbox;
        matrix_msg_info* ptr;

        puts("ID - SENDER -> DESTINATION: SUBJECT");
        puts(SEPARATOR_LINE);
        while(tmp != NULL) {
                ptr = (matrix_msg_info*)tmp->entry.data;
                printf("%u - %s -> %s: %s\n", ptr->id, ptr->sender, ptr->destination, ptr->subject);
                tmp = tmp->next;
        }

        matrix_stack_free(head_inbox);

        return 0;
}

bool sigint_pressed = false;
void sigint_handler(int code)
{
        MATRIX_LOG_INFO("SIGINT pressed");
        sigint_pressed = true;
}

int op_send()
{
        MATRIX_LOG_INFO("Send operation");

        char dest[MATRIX_USERNAME_MAX_LEN];
        char subject[MATRIX_SUBJECT_MAX_LEN];

        do {
                puts("Insert destination (empty to exit): ");
                fgets(input_buffer, INPUT_BUFFER_LEN, stdin);

        } while(strlen(input_buffer) > MATRIX_USERNAME_MAX_LEN);

        if(strlen(input_buffer) == 0) {
                return 0;
        }

        STR_PARSER_OPERATION(input_buffer)

        strncpy(dest, input_buffer, MATRIX_USERNAME_MAX_LEN);

        MATRIX_LOG_INFO("Send destination: %s", dest);

        do {
                puts("Insert subject (empty to exit): ");
                fgets(input_buffer, INPUT_BUFFER_LEN, stdin);

        } while(strlen(input_buffer) > MATRIX_SUBJECT_MAX_LEN);

        if(strlen(input_buffer) == 0) {
                return 0;
        }

        STR_PARSER_NEWLINE(input_buffer);

        strncpy(subject, input_buffer, MATRIX_SUBJECT_MAX_LEN);

        MATRIX_LOG_INFO("Send subject: %s", subject);

       
        matrix_fd fd = matrix_msg_create_draft(login_username, dest, subject);

        if(fd == -1) {
                return -1;
        }

        sigint_pressed = false;

        puts("Insert text (CONTROL-C to exit): ");
        // TODO: is it right? I mean, fgets will do the job of sigint_pressed here.
        while(!sigint_pressed) {
                // if cant read break
                // fgets will return NULL if CONTROL-C is triggered
                if(!fgets(input_buffer, INPUT_BUFFER_LEN, stdin)) {
                        break;
                }
                matrix_msg_append(fd, input_buffer);
        }
        sigint_pressed = false;
        
        matrix_file_reset_sync(fd);

        int8_t retcode = matrix_proto_put_client(&con_server, dest, subject, fd);
        
        if(retcode == MATRIX_PROTO_CODE_OK) {
                MATRIX_LOG_SHOW("Send message to server");
                return 0;
        } else if (retcode == MATRIX_PROTO_CODE_NOTFOUND) {
                MATRIX_LOG_SHOW("User not found on server");
                return 1;
        } else {
                return -1; // Unknown condition
        }
}

int op_read()
{
        MATRIX_LOG_INFO("Read operation");

        puts("Insert message ID (empty to exit): ");
        fgets(input_buffer, INPUT_BUFFER_LEN, stdin);

        if(strlen(input_buffer) == 0) {
                return 0;
        }

        errno = 0;
        matrix_msg_id id = strtol(input_buffer, NULL, 10);
        if(errno != 0) {
                MATRIX_LOG_ERRNO("Cannot read message ID");
                return -1;
        }

        matrix_fd fd = matrix_msg_receive_file(&con_server, id, login_username);

        if(fd == -1) {
                return -1;
        }

        //TODO: FIX THIS
        char buf[MATRIX_BLOCK];
        size_t nbytes = 0;
        while(nbytes = matrix_read_blk(fd, buf)) {
                matrix_write_stdout(buf, nbytes);
        }

        matrix_file_close(fd);
        return 0;
}

int op_delete()
{
        MATRIX_LOG_INFO("Delete operation");

        puts("Insert message ID (empty to exit): ");
        fgets(input_buffer, INPUT_BUFFER_LEN, stdin);

        if(strlen(input_buffer) == 0) {
                return 0;
        }

        errno = 0;
        matrix_msg_id id = strtol(input_buffer, NULL, 10);
        if(errno != 0) {
                MATRIX_LOG_ERRNO("Cannot read message ID");
                return -1;
        }

        if(matrix_proto_del_client(&con_server, id) == 0) {
                matrix_msg_delete_file(id, login_username);
                return 0;
        } else {
                return 1;
        }
}

int op_users()
{
        MATRIX_LOG_INFO("Users operation");

        char* userlist;
        matrix_stack_str* users = matrix_proto_users_client(&con_server);
        matrix_stack_str* tmp = users;
        puts("USERNAME");
        puts(SEPARATOR_LINE);
        while(tmp) {
                puts(tmp->data);
                tmp = tmp->next;
        }

        matrix_stack_str_free(users);

        return 0;
}

int operation_parse(char* str)
{
        if(strcmp(str, "help") == 0 || strcmp(str, "h") == 0) {
                return op_help();
        } else if(strcmp(str, "list") == 0 || strcmp(str, "l") == 0) {
                return op_list();
        } else if(strcmp(str, "send") == 0 || strcmp(str, "s") == 0) {
                return op_send();
        } else if(strcmp(str, "read") == 0 || strcmp(str, "r") == 0) {
                return op_read();
        } else if(strcmp(str, "delete") == 0 || strcmp(str, "d") == 0) {
                return op_delete();
        } else if(strcmp(str, "users") == 0 || strcmp(str, "u") == 0) {
                return op_users();
        } else if(strcmp(str, "quit") == 0 || strcmp(str, "q") == 0) {
                return __INT_MAX__;
        } else {
                MATRIX_LOG_INFO("Send operation");
                puts("Error: Invalid operation request!\n");
                fflush(stdout);
                return 0;
        }
}

bool ask_auth(char username[MATRIX_USERNAME_MAX_LEN], char userpass[MATRIX_USERPASS_MAX_LEN])
{
        // Ask for username
        do {
                puts("Insert username (empty to exit): ");
                fgets(input_buffer, INPUT_BUFFER_LEN, stdin);

        } while(strlen(input_buffer) > MATRIX_USERNAME_MAX_LEN);

        // TODO: Better parser for username
        STR_PARSER_OPERATION(input_buffer)

        if(strlen(input_buffer) == 0) {
                return false;
        }

        strncpy(username, input_buffer, MATRIX_USERNAME_MAX_LEN);

        // Ask for userpass
        do {
                puts("Insert userpass (empty to exit): ");
                fgets(input_buffer, INPUT_BUFFER_LEN, stdin);

        } while(strlen(input_buffer) > MATRIX_USERPASS_MAX_LEN);

        STR_PARSER_CREDENTIALS(input_buffer)

        if(strlen(input_buffer) == 0) {
                return false;
        }

        strncpy(userpass, input_buffer, MATRIX_USERPASS_MAX_LEN);

        return true;

}

void main_loop()
{
        //size_t input_size = 0;

        ssize_t nbytes_read = 0;
        int retcode = 0;

        printf("Welcome to mini-matrix\n");
        fflush(stdout);

        while (1) {
                printf("> ");
                fflush(stdout);
                fgets(input_buffer, INPUT_BUFFER_LEN, stdin);
                //input_size = strlen(input_buffer);

                STR_PARSER_OPERATION(input_buffer)

                retcode = operation_parse(input_buffer);

                if(retcode == __INT_MAX__) {
                        break;
                }
                
                if(retcode < 0 ) {
                        puts("INTERNAL ERROR!");
                }
                
        }
}

bool request_start_client(matrix_connection* con, const char* username, const char* userpass)
{
        if(!matrix_proto_handshake_client(con)) {
                return false;
        }

        if(!matrix_proto_auth_client(con, username, userpass)) {
                MATRIX_LOG_SHOW("Auth failed");
                return false;
        }

        matrix_sys_user_dir_create(username);

        MATRIX_LOG_SHOW("Auth OK");
        return true;
}



int main(int argc, char** argv)
{
        // Start the log
        MATRIX_LOG_OPEN(LOG_FILE_PATH);
        MATRIX_LOG_INFO("Starting log file %s", LOG_FILE_PATH);
        
        // if there are no arguments, take default host and port
        if(argc < 2) {
                strncpy(dest_host, STR_HOST_DEFAULT, STR_HOST_LEN);
                dest_port = NET_DEFAULT_PORT;
        } else { // if there are arguments, just take them from the os
                strncpy(dest_host, strtok(argv[1], ":"), STR_HOST_LEN);
                // TODO: Add a log with error
                // if cannot take port, just exit the program with a failure
                if(!matrix_strtou16(strtok(NULL, ":"), &dest_port)) {
                        return EXIT_FAILURE;
                }
        }

        // Setup SIGINT action
        struct sigaction sa;
        sa.sa_handler = &sigint_handler;
        sa.sa_flags = 0;
        sigfillset(&sa.sa_mask);

        sigaction(SIGINT, &sa, NULL);

        // Setup client directory tree into the OS
        matrix_sys_client_build(CLIENT_MAIN_DIR);

        // Try to connect, if fails, just exit from the program
        if(matrix_connect(&con_server, dest_host, dest_port) != 0) {
                MATRIX_LOG_ERR("Cannot connect to server!");
                exit(EXIT_FAILURE);
        }

        char login_userpass[MATRIX_USERPASS_MAX_LEN];
        if(!ask_auth(login_username, login_userpass)) {
                MATRIX_LOG_INFO("Exit, no auth provided!");
                exit(EXIT_SUCCESS);
        }


        if(!request_start_client(&con_server, login_username, login_userpass)) {
                exit(EXIT_FAILURE);
        }

        // Clean up the password memory
        memset(login_userpass, 0, MATRIX_USERPASS_MAX_LEN);

        // The User Interface Loop
        main_loop();

        // Disconnect from connection
        matrix_disconnect(&con_server);

        // Just exit with success
        return EXIT_SUCCESS;
}