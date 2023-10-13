#include "dir.h"

#include "log.h"

#include <dirent.h> // DIR, opendir, readdir
#include <sys/stat.h> // mkdir

int matrix_dir_create(const char* dir)
{
        if(mkdir(dir, MATRIX_DIR_PERMISSION) == -1) {
                if(errno != EEXIST) {
                        MATRIX_LOG_ERRNO("Cannot directory: %s", dir);
                        return -1;
                } else {
                        MATRIX_LOG_DEBUG("Access directory: %s", dir);
                        return 0;
                }
        }

        MATRIX_LOG_INFO("Created directory: %s", dir);
}

matrix_dir matrix_dir_open(const char* dir)
{
        DIR* struct_dir = opendir(dir);
        if(struct_dir == NULL) {
                MATRIX_LOG_ERRNO("Cannot open dir to count entries");
        }
        return struct_dir;
}

void matrix_dir_close(matrix_dir dir)
{
        if(closedir(dir) == -1) {
                MATRIX_LOG_ERRNO("Cannot close dir");
        }
}

ssize_t matrix_dir_entries_count(matrix_dir dir)
{
        struct dirent* direntry;

        ssize_t number_elements = 0;
        do {
                errno = 0;
                direntry = readdir(dir);
                if(direntry == NULL && errno != 0) {
                        MATRIX_LOG_ERRNO("Cannot read dir to count entries");
                        return -1;
                }
                if(direntry == NULL && errno == 0) {
                        break;
                }

                if(direntry->d_type != DT_DIR) {
                        number_elements++;
                }

        } while(1);

        matrix_dir_close(dir);

        return number_elements;
}

matrix_stack_str* matrix_dir_entries_name(matrix_dir dir)
{
        struct dirent* direntry;

        matrix_stack_str* ptr = NULL;
        do {
                errno = 0;
                direntry = readdir(dir);
                if(direntry == NULL && errno != 0) {
                        MATRIX_LOG_ERRNO("Cannot read dir to fetch entry name");
                        return NULL;
                }
                if(direntry == NULL && errno == 0) {
                        break;
                }

                if(direntry->d_type != DT_DIR) {
                        MATRIX_LOG_DEBUG("Adding directory entry to stack [entry: %s]", direntry->d_name);
                        ptr = matrix_stack_str_push(ptr, direntry->d_name);
                }

        } while(1);

        matrix_dir_close(dir);

        return ptr;
}