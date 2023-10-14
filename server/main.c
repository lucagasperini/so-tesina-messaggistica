#include <matrix.h>
#include "threads.h"
#include "request.h"

#include <stdio.h> // printf
#include <stdlib.h> // strtol, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h> // strtok_r, strcmp
#include <signal.h> // sigaction, sigfillset

#include "users.h"

#define LOG_FILE_PATH "matrix-server.log"

matrix_connection con_server;

int sys_running = 1;

void sigint_handler(int code)
{
        MATRIX_LOG_INFO("SIGINT catch");
        exit(EXIT_SUCCESS);
}

bool ask_new_user(char username[MATRIX_USERNAME_MAX_LEN], char userpass[MATRIX_USERPASS_MAX_LEN])
{
        char input_buffer[MATRIX_USERNAME_MAX_LEN];

        // Ask for username
        puts("Insert username (empty to exit): ");
        fgets(input_buffer, MATRIX_USERNAME_MAX_LEN, stdin);

        // TODO: Better parser for username
        STR_PARSER_OPERATION(input_buffer)

        if(strlen(input_buffer) == 0) {
                return false;
        }

        strncpy(username, input_buffer, MATRIX_USERNAME_MAX_LEN);


        puts("Insert userpass (empty to exit): ");
        fgets(input_buffer, MATRIX_USERNAME_MAX_LEN, stdin);

        STR_PARSER_CREDENTIALS(input_buffer)

        if(strlen(input_buffer) == 0) {
                return false;
        }

        strncpy(userpass, input_buffer, MATRIX_USERPASS_MAX_LEN);

        return true;
}


void main_loop()
{
        matrix_connection con_client;
        int thread_index = 0;

        while(sys_running)
        {

                if(matrix_accept(&con_client, &con_server) != 0) {
                        MATRIX_LOG_ERR("Cannot accept connection!");
                        continue;
                }
                threads_start(&con_client);
        }
}

void show_help()
{
        puts("Mini Matrix server\n"
             "--help show this message\n"
             "--add-user add a new user on the system\n");
}

bool new_user()
{
        char username[MATRIX_USERNAME_MAX_LEN];
        char userpass[MATRIX_USERPASS_MAX_LEN];
        if(!ask_new_user(username, userpass)){
                return false;
        }
        if(!matrix_users_add(username, userpass)) {
                return false;
        }
        return true;
}

int main(int argc, char** argv)
{
        
        MATRIX_LOG_OPEN(LOG_FILE_PATH);
        MATRIX_LOG_INFO("Starting log file %s", LOG_FILE_PATH);

        uint16_t net_port = NET_DEFAULT_PORT;

        // build server directory before anything else
        matrix_sys_server_build(SERVER_MAIN_DIR);

        if(argc != 1) {
                if(!strcmp(argv[1], "--help")) {
                        show_help();
                        return EXIT_SUCCESS;
                }
                if(!strcmp(argv[1], "--add-user")) {
                        return new_user() ? EXIT_SUCCESS : EXIT_FAILURE;
                }
                if(!matrix_strtou16(argv[1], &net_port)) {
                        return EXIT_FAILURE;
                }
        }

        //TODO: Signal management on matrix library?
        // Setup SIGINT action
        struct sigaction sa;
        sa.sa_handler = &sigint_handler;
        sa.sa_flags = 0;
        sigfillset(&sa.sa_mask);

        sigaction(SIGINT, &sa, NULL);
        
        threads_init(THREAD_NUM);

        matrix_listen(&con_server, net_port, MAX_CONNECTION);
        main_loop();
        

        return EXIT_SUCCESS;
}