/**
 * @file     esock_tcp_listener.cpp
 *           
 *
 * @author   lili  <lilijreey@gmail.com>
 * @date     01/10/2019 11:15:08 PM
 *
 */
#include <arpa/inet.h>
#include <strings.h>
#include "esock_error.hpp"
#include "esock_tcp_listener.hpp"

namespace esock {

tcp_listener_t *tcp_listener_t::make(const std::string &ip, uint16_t port)
{
    tcp_listener_t *obj = new tcp_listener_t();
    if (obj->init(ip, port) == -1) {
      delete obj;
      return nullptr;
    }
    return obj;
}

void tcp_listener_t::unmake(tcp_listener_t *&listener)
{
    delete listener;
    listener = nullptr;
}

tcp_listener_t::~tcp_listener_t()
{
  if (_listen_fd != -1)
  {
    close_socket(_listen_fd);
    _listen_fd = -1;
  }
}

int tcp_listener_t::init(const std::string &ip, uint16_t port)
{
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    int ret =
    inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr.s_addr));
    if (ret <= 0)
    {
        esock_set_syserr_msg("parse ip %s failed", ip.c_str());
        return -1;
    }

    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == _listen_fd)
    {
        esock_set_error_msg("create socket failed ");
        return -1;
    }

    if (-1 == set_sock_nonblocking(_listen_fd))
    {
        esock_set_syserr_msg("set nonblocking fd %d failed, close it", _listen_fd);
        ::close(_listen_fd);
        return -1;
    }

    if (-1 == set_sock_reuseaddr(_listen_fd))
    {
        esock_set_syserr_msg("setsockopt fd %d SO_REUSEADDR failed, close it", _listen_fd);
        ::close(_listen_fd);
        return -1;
    }


    if (-1 == bind(_listen_fd, (sockaddr*)&addr, sizeof(addr)))
    {
        esock_set_syserr_msg("bind %s %d failed", ip.c_str(), port);
        ::close(_listen_fd);
        return -1;
    }

    //sinfo init
    sockinfo_t *sinfo = sockpool.get_info(_listen_fd);
    sinfo->init(ESOCKTYPE_TCP_LISTENER); 

    _ip = ip;
    _port = port;

    return 0;
}

int tcp_listener_t::start_listen()
{
  if (-1 == ::listen(_listen_fd, 128))
  {
    sockpool.get_info(_listen_fd)->_state = ESOCKSTATE_ERROR_OCCUES;
    //TODO close it
    return -1;
  }

  sockpool.get_info(_listen_fd)->_state = ESOCKSTATE_LISTENING;

  return 0;
}


}
