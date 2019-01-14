/**
 * @file     sock_pool.cpp
 *           
 *
 * @author   lili  <lilijreey@gmail.com>
 * @date     12/17/2018 11:36:39 PM
 *
 */
#include <unistd.h>
#include <cstdlib>
#include "esock_sockpool.hpp"
#include "../esock_engine.hpp"

namespace esock { 


//extern __thread int last_errno;
//__thread char error_msg[4096];

sockpool_t sockpool;

int sockinfo_t::get_fd() const {return this - sockpool._socks;}

int sockpool_t::init() {
  if (_ref != 0)
  {
    ++_ref;
    return 0;
  }

  long open_max = sysconf(_SC_OPEN_MAX);
  if (open_max == -1)
    return -1;

  _socks = (sockinfo_t*)calloc(sizeof(sockinfo_t), open_max);
  if (_socks == nullptr)
    return -1;

  _size = open_max;
  _ref = 1;
  return 0;
}

void sockpool_t::uninit()
{
  esock_assert(_ref > 0);
  if (--_ref == 0)
  {
    free(_socks);
    _socks = 0;
    _size = 0;
  }
}



void sockinfo_t::close(int sock) {
  esock_assert(sock > 0);
  esock_assert(sock == get_fd());
  esock_assert(not is_closed());

  //remove from epoll
  if (_is_in_epoll) {
    _eng->epoll_del_sock(this);
    _eng = 0;
    _is_in_epoll = 0;
  }


  errno = 0;
  while (::close(sock) == -1 && errno == EINTR) {
    ;
  }
  if (errno != 0)
    esock_debug_log("close fd:%d failed:%s\n", sock, strerror(errno));

  _type = ESOCKTYPE_NONE;
  _state = ESOCKSTATE_CLOSED;
  _on_recvable_fn = 0;
  _on_sendable_fn = 0;
  _arg = 0;

  esock_debug_log("fd %d closed\n", sock);
}


}


