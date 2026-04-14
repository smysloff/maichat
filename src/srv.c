
// file: srv.c
// author: ishimai

#include "utils.h"

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>


#define HOST INADDR_ANY
#define PORT 6969
#define MAX_CLIENTS 64


typedef struct
{
    int fd;
    struct sockaddr_storage addr;
    socklen_t len;
    char host[NI_MAXHOST];
    char port[NI_MAXSERV];
} socket_t;


int
main(void)
{
    int server;
    int clients[MAX_CLIENTS];
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    ssize_t bytes;
    struct sockaddr_in addr;

    if ((server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    printf("Log: Server socket created (fd: %d)" "\n", server);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = HOST;
    addr.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        perror("bind()");
        close(server);
        exit(EXIT_FAILURE);
    }
    printf("Log: bound to port %d" "\n", PORT);



    close(server);
    return EXIT_SUCCESS;
}
