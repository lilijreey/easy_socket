/**
 * @file     esock_tcp_listener.hpp
 *
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/10/2019 11:00:30 PM
 *
 */

#pragma once

#include "detail/esock_sockpool.hpp"
#include "detail/esock_sysinclude_fs.hpp"

namespace esock
{

class net_engine_t;
class tcp_listener_t : detail::noncopyable_t
{
    public:
    static tcp_listener_t *make (const std::string &ip, uint16_t port);
    static void unmake (tcp_listener_t *&listener);

    public:
    int start_listen ();

    public:
    int get_sock () const { return _listen_fd; }
    const std::string &get_listen_ip () const { return _ip; }
    uint16_t get_listen_port () const { return _port; }

    public:
    // uninit
    ~tcp_listener_t ();

    private:
    int init (const std::string &ip, uint16_t port);

    std::string _ip;
    uint16_t _port = 0;
    int _listen_fd = -1;
};

static inline tcp_listener_t *make_tcp_listener (const std::string &ip, uint16_t port)
{
    return tcp_listener_t::make (ip, port);
}

static inline void unmake_tcp_listener (tcp_listener_t *&ins) { tcp_listener_t::unmake (ins); }

} // end namespace
