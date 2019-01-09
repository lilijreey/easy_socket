/**
 * @file     esock_tcp_server.hpp
 *           
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/05/2019 03:21:33 PM
 *
 */

#pragma once

#include <string>
#include <new>

#include "esock_engine.hpp"
//#include "detail/esock_utility.hpp"

namespace esock {


//class tcp_listener

//tcp_peer_conn_handle

class tcp_server_t : detail::noncopyable_t 
{
 public:
  static tcp_server_t* make(const std::string &ip, uint16_t port)
  {
    return new(std::nothrow) tcp_server_t(ip, port);
  }

  static void unmake(tcp_server_t *&svr)
  {
    delete svr;
    svr = nullptr;
  }

 private:
  tcp_server_t(const std::string &ip, uint16_t port) {
    _ip = ip;
    _port = port;

  }

 private:
  std::string _ip;
  uint16_t _port=0;
  int _listen_fd = -1;
};

} //end namespace
