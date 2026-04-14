
// file: cli.c
// author: ishimai

#include "utils.h"

#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


typedef struct
{
    char host[NI_MAXHOST];
    char port[NI_MAXSERV];
} address_t;

typedef struct
{
    int fd;
    struct sockaddr_storage addr;
    socklen_t len;
    char host[NI_MAXHOST];
    char port[NI_MAXSERV];
} socket_t;

socket_t *connect_to(address_t *address);
void socket_free(socket_t *sock);
char *get_input(const char *msg);
address_t *get_address(const char *msg);
char *get_login(const char *msg);
char *get_password(const char *msg);
address_t *address_extract(const char *raw);


int
main(void)
{
    socket_t *server;
    FILE *stream;
    address_t *address;
    //char *login, *password; // @todo after server auth system
    fd_set rset;
    int current;
    int maxfd;
    char *message;
    char buffer[4096];
    ssize_t bytes;

    while (1)
    {
        while (!(address = get_address("address: ")))
            fprintf(stderr, "Error: Invalid address format\n");

        if ((server = connect_to(address)))
            break;

        fprintf(stderr, "Error: Cannot connect to %s:%s\n",
            address->host, address->port);

        free(address);
        address = NULL;
    }

    maxfd = imax(STDIN_FILENO, server->fd) + 1;

    while (1)
    {
        FD_ZERO(&rset);
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(server->fd, &rset);

        if (select(maxfd, &rset, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &rset))
        {
            if ((message = fgetline(stdin)))
            {
                send(server->fd, message, strlen(message), 0);
                free(message);
            }
            fflush(stdin);
        }

        if (FD_ISSET(server->fd, &rset))
        {
            bytes = recv(server->fd, buffer, sizeof(buffer) - 1, 0);
            // if (bytes == -1) @todo
            // if (bytes == 0)  @todo
            buffer[bytes] = '\0';
            fprintf(stdout, "%s", buffer);
            fflush(stdout);
        }

    }


    // @todo after server auth system

    //while (!(login = get_login("login: ")))
    //    fprintf(stderr, "Error: Wrong login. Type correct user login\n");

    //while (!(password = get_password("password: ")))
    //    fprintf(stderr, "Error: Wrong password. Type correct user password\n");

    //printf("address: %s:%s\n", address->host, address->port);
    //printf("login: %s\n", login);
    //printf("password: %s\n", password);


    //if (!(sock = connect_to(address)))
    //{
    //    const char *msg = smprintf("Cannot connect to %s:%s", address->host, address->port);
    //    free(address);
    //    //free(login);
    //    //free(password);
    //    die(msg);
    //}

    const char buf[] = "Hello, World!\n";
    bytes = send(server->fd, buf, sizeof(buf), 0);

    if (bytes == -1)
        perror("send()");

    socket_free(server);
    free(address);
    //free(login);
    //free(password);

    return EXIT_SUCCESS;
}


socket_t *
connect_to(address_t *address)
{
    socket_t *sock;
    struct addrinfo hints;
    struct addrinfo *addrs, *addr;
    int err;

    if (!address)
        return NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(address->host, address->port, &hints, &addrs) != 0)
        return NULL;

    if (!(sock = malloc(sizeof(*sock))))
        return NULL;

    for (addr = addrs; addr; addr = addr->ai_next)
    {
        sock->fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        if (sock->fd == -1)
            continue;

        if (connect(sock->fd, addr->ai_addr, addr->ai_addrlen) == 0)
            break;

        close(sock->fd);
        sock->fd = -1;
    }

    if (sock->fd != -1)
    {
        memcpy(&sock->addr, addr->ai_addr, addr->ai_addrlen);
        sock->len = addr->ai_addrlen;

        err = getnameinfo(
            (struct sockaddr *) &sock->addr, sock->len,
            sock->host, sizeof(sock->host),
            sock->port, sizeof(sock->port),
            NI_NUMERICHOST | NI_NUMERICSERV
        );

        if (err)
        {
            socket_free(sock);
            freeaddrinfo(addrs);
            return NULL;
        }
    }

    else
    {
      socket_free(sock);
      freeaddrinfo(addrs);
      return NULL;
    }

    freeaddrinfo(addrs);
    return sock;
}

void
socket_free(socket_t *sock)
{
  if (!sock)
    return;
  if (sock->fd >= 0)
    close(sock->fd);
  free(sock);
}

char *
get_input(const char *msg)
{
    char *raw, *result;

    if (msg)
        printf("%s", msg);

    if (!(raw = fgetline(stdin)))
        return NULL;

    result = trim(raw);
    free(raw);

    return result;
}

address_t *
get_address(const char *msg)
{
    address_t *address;
    char *raw;

    raw = get_input(msg);
    if (!raw || *raw == '\0' || has_spaces(raw))
    {
        free(raw);
        return NULL;
    }

    address = address_extract(raw);
    free(raw);

    return address; // nullable
}

char *
get_login(const char *msg)
{
    char *login;

    login = get_input(msg);
    if (
        !login
     || *login == '\0'
     || strlen(login) < 2
     || !isalpha((unsigned char) *login)
     || has_spaces(login)
    ) {
        free(login);
        return NULL;
    }

    return login;
}

char *
get_password(const char *msg)
{
    char *password;

    password = get_input(msg);
    if (
        !password
     || *password == '\0'
     || strlen(password) < 8
     || has_spaces(password)
    ) {
        free(password);
        return NULL;
    }

    return password;
}

address_t *
address_extract(const char *raw)
{
    address_t *address;
    const char *delim;
    const char *host, *port;
    size_t hostlen, portlen;

    if (!raw)
        return NULL;

    if (!(delim = strrchr(raw, ':')))
        return NULL;

    host = raw;
    port = delim + 1;

    if (*port == '\0')
        return NULL;

    if (!(address = malloc(sizeof *address)))
        return NULL;

    hostlen = delim - host;

    if (hostlen >= 2 && host[0] == '[' && host[hostlen - 1] == ']')
    {
        ++host;
        hostlen -= 2;
    }

    if (hostlen == 0)
    {
        free(address);
        return NULL;
    }

    if (hostlen >= sizeof(address->host))
        hostlen = sizeof(address->host) - 1;

    strncpy(address->host, host, hostlen);
    address->host[hostlen] = '\0';

    portlen = strlen(port);

    if (portlen >= sizeof(address->port))
        portlen = sizeof(address->port) - 1;

    strncpy(address->port, port, portlen);
    address->port[portlen] = '\0';

    return address;
}
