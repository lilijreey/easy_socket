/**
 * @file     esock_socket.hpp
 *
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/05/2019 08:19:34 PM
 *
 */

#pragma once

#include "detail/esock_sysinclude_fs.hpp"

namespace esock
{

/**
 * @return false means invaild ip.
 */
static inline bool init_sockaddr_in (sockaddr_in &addr, const std::string &ip, uint16_t port)
{
    ::bzero (&addr, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons (port);
    return 1 == inet_pton (AF_INET, ip.c_str (), &(addr.sin_addr.s_addr));
}

static inline int set_sock_nonblocking (int sock)
{
    int flags = fcntl (sock, F_GETFL, 0);
    if (-1 == flags) return -1;

    flags |= O_NONBLOCK;

    return fcntl (sock, F_SETFL, flags);
}

static inline int get_sock_error (int fd)
{
    int val;
    socklen_t len = sizeof (val);
    if (-1 == getsockopt (fd, SOL_SOCKET, SO_ERROR, &val, &len))
    {
        return -1;
    }
    return val;
}

static inline int set_sock_reuseaddr (int sock)
{
    int opt = 1;
    return setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt));
}

static inline int set_sock_nodelay (int sock)
{
    int val = 1;
    return setsockopt (sock, IPPROTO_TCP, TCP_NODELAY, &val, sizeof (val));
}

static inline std::pair<bool, sockaddr_storage> get_sock_localaddr (int sock)
{
    sockaddr_storage addr;
    socklen_t addrlen = sizeof (addr);

    if (::getsockname (sock, (sockaddr *)&addr, &addrlen) == -1) return { false, addr };

    return { true, addr };
}

static inline std::pair<bool, sockaddr_storage> get_sock_peeraddr (int sock)
{
    sockaddr_storage addr;
    socklen_t addrlen = sizeof (addr);

    if (::getpeername (sock, (sockaddr *)&addr, &addrlen) == -1) return { false, addr };

    return { true, addr };
}

static inline std::string get_sockaddr_ip (const sockaddr_storage &addr)
{
    char buf[INET6_ADDRSTRLEN] = { 0 };

    if (addr.ss_family == AF_INET)
    {
        inet_ntop (addr.ss_family, &(((sockaddr_in *)&addr)->sin_addr), buf, INET_ADDRSTRLEN);
    }
    else if (addr.ss_family == AF_INET6)
    {
        inet_ntop (addr.ss_family, &(((sockaddr_in6 *)&addr)->sin6_addr), buf, INET6_ADDRSTRLEN);
    }

    return buf;
}

static inline int get_sockaddr_port (const sockaddr_storage &addr)
{
    if (addr.ss_family == AF_INET)
    {
        return ntohs (((sockaddr_in *)&addr)->sin_port);
    }

    //(addr.ss_family == AF_INET6)
    return ntohs (((sockaddr_in6 *)&addr)->sin6_port);
}

} // end namespace
