#include "file.h"
#include "log.h"

#include <unistd.h> // write, read, close
#include <fcntl.h> // open, SEEK_CUR
#include <string.h> //strlen

matrix_fd matrix_file_open_oflag_perm(const char* file, int oflag, int perm)
{
        matrix_fd fd = -1;
        
        do {
                errno = 0;
                fd = open(
                        file, 
                        oflag, 
                        perm
                );

                if(fd == -1 && errno != EINTR) {
                        MATRIX_LOG_ERRNO("Cannot open file %s", file);
                        return -1;
                }

        } while(fd == -1);

        return fd;

}

matrix_fd matrix_file_open_oflag(const char* file, int oflag)
{
        matrix_fd fd = -1;
        
        do {
                errno = 0;
                fd = open(
                        file, 
                        oflag
                );

                if(fd == -1 && errno != EINTR) {
                        MATRIX_LOG_ERRNO("Cannot open file %s", file);
                        return -1;
                }

        } while(fd == -1);

        return fd;

}


matrix_fd matrix_file_open_new(const char* file)
{
        return matrix_file_open_oflag_perm(
                file, 
                MATRIX_DEFAULT_FILE_FLAGS | O_TRUNC, 
                MATRIX_DEFAULT_FILE_PERMISSION
        );
}

matrix_fd matrix_file_open(const char* file)
{
        return matrix_file_open_oflag_perm(
                file, 
                MATRIX_DEFAULT_FILE_FLAGS,
                MATRIX_DEFAULT_FILE_PERMISSION
        );
}

matrix_fd matrix_file_open_ro(const char* file)
{
        return matrix_file_open_oflag(
                file, 
                O_RDONLY
        );
}


matrix_fd matrix_file_open_wo(const char* file)
{
        return matrix_file_open_oflag_perm(
                file, 
                O_CREAT | O_WRONLY,
                MATRIX_DEFAULT_FILE_PERMISSION
        );
}

matrix_fd matrix_file_open_wot(const char* file)
{
        return matrix_file_open_oflag_perm(
                file, 
                O_CREAT | O_WRONLY | O_TRUNC,
                MATRIX_DEFAULT_FILE_PERMISSION
        );
}


bool matrix_file_copy(const char* src_path, const char* dest_path)
{
        // TODO: Add return false
        matrix_fd src_fd = matrix_file_open_ro(src_path);
        matrix_file_copy_fd(src_fd, dest_path);
        matrix_file_close(src_fd);

        return true;
}

bool matrix_file_copy_fd(matrix_fd src_fd, const char* dest_path)
{
        MATRIX_ASSERT_VALID_FD(src_fd)
        matrix_file_reset(src_fd);
        // TODO: Add return false
        matrix_fd dest_fd = matrix_file_open_wot(dest_path);

        char buf[MATRIX_BLOCK];
        ssize_t nbytes_read = 0;
        while((nbytes_read = matrix_read_blk(src_fd, buf)) > 0) {
                matrix_write(dest_fd, buf, nbytes_read);
        }

        matrix_file_close(dest_fd);

        return true;
}

size_t matrix_read(matrix_fd fd, void* buf, size_t sz)
{
        MATRIX_ASSERT_VALID_FD(fd)
        ssize_t nbyte = -1;
        do {
                errno = 0;
                nbyte = read(fd, buf, sz);
                if(nbyte == -1 && errno != EINTR) {
                        MATRIX_LOG_ERRNO("Cannot read fd %d", fd);
                        return -1;
                }
        } while(nbyte == -1);

        return nbyte;
}

size_t matrix_read_line(matrix_fd fd, char* buf, size_t sz)
{
        char c;
        int i = 0;
        for(; matrix_read_char(fd, &c) && i + 1 < sz; i++) {
                buf[i] = c;
                if(c == '\n') {
                        buf[i + 1] = '\0';
                        return i + 1;
                }
        }

        // cant read because buffer is not big enough
        if(i > sz) {
                MATRIX_LOG_ERR("Cant receive line, buffer too small [sz: %d]", sz);
        }
        
        return 0;
}

size_t matrix_read_string(matrix_fd fd, char* buf, size_t sz)
{
        char c;
        int i = 0;
        for(; matrix_read_char(fd, &c) && i + 1 < sz; i++) {
                buf[i] = c;
                if(c == '\0') {
                        return i + 1;
                }
        }

        // cant read because buffer is not big enough
        if(i > sz) {
                MATRIX_LOG_ERR("Cant receive string, buffer too small [sz: %d]", sz);
        }

        // cant read because buffer is not big enough
        return 0;
}

size_t matrix_write(matrix_fd fd, const void* buf, size_t sz)
{
        MATRIX_ASSERT_VALID_FD(fd)
        ssize_t nbyte = -1;
        do {
                nbyte = write(fd, buf, sz);
                if(nbyte == -1 && errno != EINTR) {
                        MATRIX_LOG_ERRNO("Cannot write fd %d", fd);
                        return 0;
                }
        } while(nbyte == -1);

        return nbyte;
}

size_t matrix_write_stdout(const void* mem, size_t sz)
{
        return matrix_write(STDOUT_FILENO, mem, sz);
}

size_t matrix_write_stderr(const void* mem, size_t sz)
{
        return matrix_write(STDERR_FILENO, mem, sz);
}

size_t matrix_write_line(matrix_fd fd, const char* buf)
{
        ssize_t nbyte = matrix_write(fd, buf, strlen(buf));
        nbyte += matrix_write_char(fd, '\n');
        return nbyte;
}

size_t matrix_write_string(matrix_fd fd, const char* buf)
{
        return matrix_write(fd, buf, strlen(buf));
}

matrix_offset matrix_file_seek(matrix_fd fd, matrix_offset offset, int mode)
{
        MATRIX_ASSERT_VALID_FD(fd)
        matrix_offset pos = -1;
        if((pos = lseek(fd, 0, mode)) == -1) {
                MATRIX_LOG_ERRNO("Cannot seek file");
        }

        return pos;
}

matrix_offset matrix_file_seek_start(matrix_fd fd, matrix_offset offset)
{
        return matrix_file_seek(fd, offset, SEEK_SET);
}

matrix_offset matrix_file_seek_current(matrix_fd fd, matrix_offset offset)
{
        return matrix_file_seek(fd, offset, SEEK_CUR);
}

matrix_offset matrix_file_seek_end(matrix_fd fd, matrix_offset offset)
{
        return matrix_file_seek(fd, offset, SEEK_END);
}

void matrix_file_sync(matrix_fd fd)
{
        MATRIX_ASSERT_VALID_FD(fd)
        int ret = -1;
        do {
                errno = 0;
                ret = fsync(fd);
                if(ret == -1 && errno != EINTR) {
                        MATRIX_LOG_ERRNO("Cannot sync file");
                        return;
                }

        } while(ret == -1);
}

void matrix_file_close(matrix_fd fd)
{
        MATRIX_ASSERT_VALID_FD(fd)
        int ret = -1;
        do {
                errno = 0;
                ret = close(fd);
                if(ret == -1 && errno != EINTR) {
                        MATRIX_LOG_ERRNO("Cannot close file");
                        return;
                }
        } while(ret == -1);
}

void matrix_file_truncate(matrix_fd fd, size_t offset)
{
        MATRIX_ASSERT_VALID_FD(fd)
        int ret = -1;
        do {
                errno = 0;
                ret = ftruncate(fd, offset);
                if(ret == -1 && errno != EINTR) {
                        MATRIX_LOG_ERRNO("Cannot truncate file");
                        return;
                }
        } while(ret == -1);
}



bool matrix_file_parse_name(const char* fullpath, char* buf)
{
        const char* i = fullpath;
        const char* first = NULL;
        const char* last = NULL;

        while(*i) {
                if(*i == '/') {
                        first = i + 1;
                } else if(*i == '.') {
                        last = i;
                }
                i++;
        }
        if(first == NULL) {
                return false;
        }

        if(last != NULL) {
                if(first > last) {
                        strncpy(buf, first, i - first);
                        buf[i - first] = '\0';  // add null-terminator
                } else {
                        strncpy(buf, first, last - first);
                        buf[last - first] = '\0';  // add null-terminator
                }
        } else {
                strncpy(buf, first, i - first); 
                buf[i - first] = '\0';  // add null-terminator
        }
        return true;
}

bool matrix_file_delete(const char* fullpath)
{
        if(unlink(fullpath) == -1) {
                MATRIX_LOG_ERRNO("Cannot delete file [file: %s]", fullpath);
                return false;
        }

        return true;
}