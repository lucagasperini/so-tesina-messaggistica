#pragma once

#include <errno.h> // errno
#include <stdlib.h> // NULL, getenv

#define MATRIX_LOG_OPEN(file) matrix_log_open(file)


void matrix_log_open(const char* file);
void matrix_log_show(const char* msg, ...);
void matrix_log_debug(const char* msg, ...);
void matrix_log_info(const char* msg, ...);
void matrix_log_error(const char* msg, ...);

#define _MATRIX_LOG_SHOW(msg, ...) matrix_log_show(msg "\n", __VA_ARGS__)
#define MATRIX_LOG_SHOW(...) _MATRIX_LOG_SHOW(__VA_ARGS__, "")

// CMake define this when is not debug build
#ifndef NDEBUG  // is on debug

#define _MATRIX_LOG_MALLOC(msg, ...) \
        matrix_log_debug("[MEM/ALLOC] " msg " [var: %s, ptr: %p, sz: %d]\n", __VA_ARGS__)
#define _MATRIX_LOG_CALLOC(msg, ...) \
        matrix_log_debug("[MEM/CALLOC] " msg " [var: %s, ptr: %p, num: %d, sz: %d]\n", __VA_ARGS__)
#define _MATRIX_LOG_FREE(msg, ...) \
        matrix_log_debug("[MEM/FREE] " msg " [var: %s, ptr: %p]\n", __VA_ARGS__)
#define _MATRIX_LOG_REALLOC(msg, ...) \
        matrix_log_debug("[MEM/REALLOC] " msg " [var: %s, ptr: %p, sz: %d]\n", __VA_ARGS__)

#define _MATRIX_LOG_NET_FROM(con, msg, ...) {                                           \
        char addr[INET_ADDRSTRLEN];                                                     \
        inet_ntop(AF_INET, &(con->net_addr->sin_addr), addr, INET_ADDRSTRLEN);          \
        uint16_t port = ntohs(con->net_addr->sin_port);                                 \
        matrix_log_debug("[NET] " msg " [from: %s:%hu]\n", __VA_ARGS__, addr, port);    \
}

#define _MATRIX_LOG_NET_TO(con, msg, ...) {                                             \
        char addr[INET_ADDRSTRLEN];                                                     \
        inet_ntop(AF_INET, &(con->net_addr->sin_addr), addr, INET_ADDRSTRLEN);          \
        uint16_t port = ntohs(con->net_addr->sin_port);                                 \
        matrix_log_debug("[NET] " msg " [to: %s:%hu]\n", __VA_ARGS__, addr, port);      \
}

#define _MATRIX_LOG_DEBUG(msg, ...) matrix_log_debug("[DEBUG] " msg "\n", __VA_ARGS__)
#define _MATRIX_LOG_INFO(msg, ...) matrix_log_debug("[INFO] " msg "\n", __VA_ARGS__)

#else           // is not on debug

#define _MATRIX_LOG_MALLOC(msg, ...)
#define _MATRIX_LOG_CALLOC(msg, ...)
#define _MATRIX_LOG_FREE(msg, ...)
#define _MATRIX_LOG_REALLOC(msg, ...)

#define _MATRIX_LOG_NET_FROM(con, msg, ...) 
#define _MATRIX_LOG_NET_TO(con, msg, ...) 

#define _MATRIX_LOG_DEBUG(msg, ...)
#define _MATRIX_LOG_INFO(msg, ...) matrix_log_info("[INFO] " msg "\n", __VA_ARGS__)

#endif

#define MATRIX_LOG_NET_FROM(con, msg, ...) _MATRIX_LOG_NET_FROM(con, msg, __VA_ARGS__)
#define MATRIX_LOG_NET_TO(con, msg, ...) _MATRIX_LOG_NET_TO(con, msg, __VA_ARGS__)

#define MATRIX_LOG_DEBUG(...) _MATRIX_LOG_DEBUG(__VA_ARGS__, "")
#define MATRIX_LOG_INFO(...) _MATRIX_LOG_INFO(__VA_ARGS__, "")

#define _MATRIX_LOG_ERR(msg, ...) matrix_log_error("[ERROR] " msg "\n", __VA_ARGS__)
#define MATRIX_LOG_ERR(...) _MATRIX_LOG_ERR(__VA_ARGS__, "")

#define _MATRIX_LOG_ERRNO(msg, ...) matrix_log_error("[ERROR] " msg " [errno: %d]\n", __VA_ARGS__)
#define MATRIX_LOG_ERRNO(...) _MATRIX_LOG_ERRNO(__VA_ARGS__, errno)

#define MATRIX_MALLOC(var, sz, ...)                                             \
        var = malloc(sz);                                                       \
        if(var == NULL)                                                         \
                MATRIX_LOG_ERR("malloc failed [var: %s, sz: %d]", #var, sz);    \
        _MATRIX_LOG_MALLOC(__VA_ARGS__, #var, var, sz);

#define MATRIX_CALLOC(var, num, sz, ...)                                                        \
        var = calloc(num, sz);                                                                  \
        if(var == NULL)                                                                         \
                MATRIX_LOG_ERR("calloc failed [var: %s, num: %d, sz: %d]", #var, num, sz);      \
        _MATRIX_LOG_CALLOC(__VA_ARGS__, #var, var, num, sz);

#define MATRIX_FREE(var, ...)                           \
        free(var);                                      \
        _MATRIX_LOG_FREE(__VA_ARGS__, #var, var);

#define MATRIX_REALLOC(var, sz, ...)                                            \
        var = realloc(var, sz);                                                 \
        if(var == NULL)                                                         \
                MATRIX_LOG_ERR("realloc failed [var: %s, sz: %d]", #var, sz);   \
        _MATRIX_LOG_REALLOC(__VA_ARGS__, #var, var, sz);


#define _MATRIX_STRINGIZE2(x) #x
#define _MATRIX_STRINGIZE(x) _MATRIX_STRINGIZE2(x)
#define MATRIX_LOG_LINE __FILE__":"_MATRIX_STRINGIZE(__LINE__)

#define _MATRIX_ASSERT(cond, msg, ...) if(!(cond)) { matrix_log_error("[ASSERT] (" #cond ") at (" MATRIX_LOG_LINE ")" msg "\n", __VA_ARGS__); exit(EXIT_FAILURE); }
#define MATRIX_ASSERT(cond, ...) _MATRIX_ASSERT(cond, __VA_ARGS__, "")