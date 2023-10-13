#include "log.h"

#include <ctype.h> // tolower
#include <fcntl.h> // open
#include <stdio.h> // fprintf, vdprintf
#include <stdarg.h> // va_list, va_start, va_end
#include <unistd.h> // fsync
#include <pthread.h> // pthread_mutex_lock, pthread_mutex_unlock

int matrix_log_fd = -1;
static pthread_mutex_t s_matrix_log_mutex;


void matrix_log_open(const char* file)
{
        errno = 0;
        // try to open log file
        matrix_log_fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0600);
        // if cannot be open, then write errno and quit this program
        if(matrix_log_fd == -1) {
                fprintf(stderr, "Cannot open log file at %s: [errno: %d]\n", file, errno);
                exit(EXIT_FAILURE);
        }
        // then init log mutex
        pthread_mutex_init(&s_matrix_log_mutex, NULL);
}

void matrix_log_show(const char* msg, ...)
{
        va_list args;
        // lock the log mutex
        pthread_mutex_lock(&s_matrix_log_mutex);

        // set args for matrix log
        va_start(args, msg);
        dprintf(matrix_log_fd, "[SHOW] ");
        vdprintf(matrix_log_fd, msg, args);
        va_end(args);

        // set args for stdout
        va_start(args, msg);
        vdprintf(STDOUT_FILENO, msg, args);
        va_end(args);

        // then flush both
        fsync(matrix_log_fd);
        fsync(STDOUT_FILENO);
        // unlock the log mutex
        pthread_mutex_unlock(&s_matrix_log_mutex);
}

void matrix_log_debug(const char* msg, ...)
{
        va_list args;

        // lock the log mutex
        pthread_mutex_lock(&s_matrix_log_mutex);
        // set args for matrix log
        va_start(args, msg);
        vdprintf(matrix_log_fd, msg, args);
        va_end(args);

        // set args for stdout
        va_start(args, msg);
        vdprintf(STDOUT_FILENO, msg, args);
        va_end(args);

        // then flush both
        fsync(matrix_log_fd);
        fsync(STDOUT_FILENO);

        // unlock the log mutex
        pthread_mutex_unlock(&s_matrix_log_mutex);
}

void matrix_log_info(const char* msg, ...)
{
        va_list args;
        // lock the log mutex
        pthread_mutex_lock(&s_matrix_log_mutex);

        va_start(args, msg);
        vdprintf(matrix_log_fd, msg, args);
        va_end(args);
        fsync(matrix_log_fd);

        // unlock the log mutex
        pthread_mutex_unlock(&s_matrix_log_mutex);
}

void matrix_log_error(const char* msg, ...)
{
        va_list args;
        // lock the log mutex
        pthread_mutex_lock(&s_matrix_log_mutex);

        // set args for matrix log
        va_start(args, msg);
        vdprintf(matrix_log_fd, msg, args);
        va_end(args);

        // set args for stderr
        va_start(args, msg);
        vdprintf(STDERR_FILENO, msg, args);
        va_end(args);

        // then flush both
        fsync(matrix_log_fd);
        fsync(STDERR_FILENO);

        // unlock the log mutex
        pthread_mutex_unlock(&s_matrix_log_mutex);
}

