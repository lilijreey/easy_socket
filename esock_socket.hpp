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

namespace esock {


static inline int set_socket_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (-1 == flags)
      return -1;

    flags |= O_NONBLOCK;

    return fcntl(sock, F_SETFL, flags);
}

static inline int get_socket_error(int fd)
{
    int val;
    socklen_t len = sizeof(val);
    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &val, &len))
    {
        return -1;
    }
    return val;
}

//TODO
//get_local_addr();
//get_peer_addr();


}//end namespace
