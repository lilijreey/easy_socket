/**
 * @file     esock_socket.hpp
 *           
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/05/2019 08:19:34 PM
 *
 */


#pragma once


#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

namespace esock {


static inline int set_sock_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (-1 == flags)
      return -1;

    flags |= O_NONBLOCK;

    return fcntl(sock, F_SETFL, flags);
}

static inline int get_sock_error(int fd)
{
    int val;
    socklen_t len = sizeof(val);
    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &val, &len))
    {
        return -1;
    }
    return val;
}

static inline int set_sock_reuseaddr(int sock)
{
  int opt = 1;
  return setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, &opt, sizeof(opt));
}

static inline int set_sock_nodelay(int sock)
{
    int val = 1;
    return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
}

//TODO
//get_local_addr();
//get_peer_addr();


}//end namespace
