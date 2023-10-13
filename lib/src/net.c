#include "net.h"
#include "log.h"
#include "file.h"
#include "utils.h"

#include <stdio.h> // fprintf, stderr
#include <sys/socket.h> // socket, setsockopt, bind, listen, accept
#include <netinet/in.h> //sockaddr_in, htonl, htons
#include <netdb.h> // gethostbyname
#include <arpa/inet.h> // inet_addr, inet_ntoa
#include <stdlib.h> // NULL
#include <unistd.h> // write, read, close
#include <errno.h> // errno
#include <string.h> // memset, memcpy

int matrix_connect(matrix_connection* con, char dest_host[STR_HOST_LEN], uint16_t dest_port)
{
        con->fd = socket(
                AF_INET,                /* IPv4 Internet protocols */
                SOCK_STREAM,            /* Provides sequenced, reliable, two-way, connection-based byte streams. */
                0                       /* protocol: protoent->p_proto */
        );

        if(con->fd == -1) {
                MATRIX_LOG_ERRNO("Cannot create a socket file descriptor!");
                return 1;
        }

        /* Prepare sockaddr_in. */
        struct hostent* hent = gethostbyname(dest_host);
        if (hent == NULL) {
                MATRIX_LOG_ERRNO("Cannot resolve host \"%s\"", dest_host);
                return 2;
        }
        in_addr_t bin_net_addr = inet_addr(inet_ntoa(*(struct in_addr*)*(hent->h_addr_list)));
        if (bin_net_addr == (in_addr_t)-1) {
                MATRIX_LOG_ERRNO("Cannot convert to binary \"%s\"", *(hent->h_addr_list));
                return 3;
        }

        MATRIX_MALLOC(con->net_addr, sizeof(struct sockaddr_in), "Connect server address");
        memset(con->net_addr, 0, sizeof(struct sockaddr_in));

        con->net_addr->sin_addr.s_addr = bin_net_addr;
        con->net_addr->sin_family = AF_INET;
        con->net_addr->sin_port = htons(dest_port);

        if (connect(con->fd, (struct sockaddr*)con->net_addr, sizeof(struct sockaddr_in)) == -1) {
                MATRIX_LOG_ERRNO("Cannot connect!");
                return 4;
        }

        return 0;
}

void matrix_listen(matrix_connection* con, uint16_t net_port, size_t max_connections)
{
        con->fd = socket(
                AF_INET,                /* IPv4 Internet protocols */
                SOCK_STREAM,            /* Provides sequenced, reliable, two-way, connection-based byte streams. */
                0                       /* protocol: protoent->p_proto */
        );

        if(con->fd == -1) {
                MATRIX_LOG_ERRNO("Cannot create a socket file descriptor!");
                exit(EXIT_FAILURE);
        }

        {
                int yes = 1;
                setsockopt(con->fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        }

        MATRIX_MALLOC(con->net_addr, sizeof(struct sockaddr_in), "Listen server address");
        memset(con->net_addr, 0, sizeof(struct sockaddr_in));

        con->net_addr->sin_family = AF_INET;
        con->net_addr->sin_addr.s_addr = htonl(INADDR_ANY);
        con->net_addr->sin_port = htons(net_port);

        if (bind(con->fd, (struct sockaddr*)con->net_addr, sizeof(struct sockaddr_in)) == -1)
        {
                MATRIX_LOG_ERRNO("Cannot assign address to socket file descriptor!");
                exit(EXIT_FAILURE);
        }

        if (listen(con->fd, max_connections) == -1) {
                MATRIX_LOG_ERRNO("Cannot listen to socket!");
                exit(EXIT_FAILURE);
        }

        MATRIX_LOG_SHOW("Messaging server is listening on port %d", net_port);
}

int matrix_accept(matrix_connection* client, matrix_connection* server)
{
        socklen_t client_len = sizeof(struct sockaddr_in);

        MATRIX_MALLOC(client->net_addr, client_len, "Allocate input connection struct");
        memset(client->net_addr, 0, client_len);

        int ret = -1;
        do {
                errno = 0;
                ret = accept(
                        server->fd,
                        (struct sockaddr*)client->net_addr,
                        &client_len
                );

                if(client->fd == -1 && errno != EINTR) {
                        MATRIX_LOG_ERRNO("Cannot accept connection");
                        return -1;
                }
        } while(ret == -1);

        client->fd = ret;

        return 0;
}

void matrix_disconnect(matrix_connection* con)
{
        matrix_file_close(con->fd);
        MATRIX_FREE(con->net_addr, "Disconnection");
        memset(con, 0, sizeof(matrix_connection));
}

void matrix_copy_connection(matrix_connection* dest, const matrix_connection* src)
{
        dest->fd = src->fd;
        MATRIX_MALLOC(dest->net_addr, sizeof(struct sockaddr_in), "Copy connection");
        memcpy(dest->net_addr, src->net_addr, sizeof(struct sockaddr_in));
}