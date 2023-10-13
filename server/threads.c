#include "threads.h"
#include "log.h"
#include "net.h"
#include "request.h"

#include <stdlib.h> // NULL, calloc
#include <pthread.h> // pthread_t, pthread_create


client_thread_info* matrix_threads_info = NULL;
/**
 * @brief List of thread id generated
 * 
 */
pthread_t* matrix_threads_ids = NULL;

int matrix_threads_num = 0;

void threads_init(int num_threads)
{
        matrix_threads_num = num_threads;
        MATRIX_CALLOC(matrix_threads_ids, num_threads, sizeof(pthread_t), "Thread id array");
        MATRIX_CALLOC(matrix_threads_info, num_threads, sizeof(client_thread_info), "Thread info array");
        for(int i = 0; i < num_threads; i++) {
                matrix_threads_info[i].index = i;
        }
}

void* thread_routine(void* args)
{
        client_thread_info* tinfo = (client_thread_info*)args;
        if(!request_start_server(tinfo)) {
                return NULL;
        }

        request_loop(tinfo);
        matrix_disconnect(&(tinfo->con));
        tinfo->status = 0;
}

bool threads_start(matrix_connection* con)
{
        for(int i = 0; i < matrix_threads_num; i++) {
                if(matrix_threads_info[i].status == 0) {
                        matrix_threads_info[i].status = 1;
                        MATRIX_LOG_SHOW("Accepting connection %d to thread %d", con->fd, i);

                                matrix_copy_connection(
                                        &(matrix_threads_info[i].con),
                                        con
                                );

                                if(pthread_create(
                                        &(matrix_threads_ids[i]), 
                                        NULL, 
                                        thread_routine, 
                                        (void*)&(matrix_threads_info[i])
                                ) != 0) {
                                        MATRIX_LOG_ERR("Cannot generate thread!");
                                        return false;
                                }
                                return true;
                        }
        }

        MATRIX_LOG_ERR("Cannot handle this connection request!");
        return false;
}