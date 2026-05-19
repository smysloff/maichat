
// file: lib/socket.h

#pragma once

#include <sys/socket.h>  // socket(), connect(), bind(), listen(), accept(), recv(), send()
                         // sockaddr, sockaddr_storage
                         // sa_family_t, socklen_t

#include <netinet/in.h>  // sockaddr_in, in_port_t, in_addr, in_addr_t
                         // sockaddr_in6, in6_addr

int Socket(int domain, int type, int protocol);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t Recv(int sockfd, void *buf, size_t size, int flags);
ssize_t Send(int sockfd, const void *buf, size_t size, int flags);
