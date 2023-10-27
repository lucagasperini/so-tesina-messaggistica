#pragma once

#include "log.h"

#include <stdlib.h> // strtol
#include <stdbool.h> // true, false

// TODO: Check for overflow
// makes all characters lower case and remove endline
#define STR_PARSER_OPERATION(str)                       \
        {                                               \
                int i = 0;                              \
                for(; str[i]; i++)                      \
                        str[i] = tolower(str[i]);       \ 
                str[i - 1] = '\0';                      \
        }

// TODO: Check for overflow
// remove endline
#define STR_PARSER_CREDENTIALS(str)                     \
                str[strlen(str) - 1] = '\0';

#define STR_PARSER_NEWLINE(str) str[strlen(str) - 1] = '\0';

static inline bool matrix_strtol(const char* str, long* res)
{
        errno = 0;
        long l = strtol(str, NULL, 10);
        if(errno != 0) {
                MATRIX_LOG_ERRNO("Cannot parse string to long! [str: %s]", str);
                return false;
        }
        *res = l;

        return true;
}

static inline bool matrix_strtou32(const char* str, uint32_t* res)
{
        long l;
        if(!matrix_strtol(str, &l)) {
                return false;
        }

        if(l > UINT32_MAX) {
                MATRIX_LOG_ERR("Cannot convert long to u32! [long: %li]", l);
                return false;
        }

        *res = l;
        return true;
}

static inline bool matrix_strtou16(const char* str, uint16_t* res)
{
        long l;
        if(!matrix_strtol(str, &l)) {
                return false;
        }

        if(l > UINT16_MAX) {
                MATRIX_LOG_ERR("Cannot convert long to u16! [long: %li]", l);
                return false;
        }

        *res = l;
        return true;
}