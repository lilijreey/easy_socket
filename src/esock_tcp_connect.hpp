/**
 * @file     esock_async_tcp_conn_handler.hpp
 *           
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/05/2019 04:54:14 PM
 *
 */

#pragma once


#include <utility> //pair
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "detail/esock_sockpool.hpp"
#include "detail/esock_sockpool.hpp"

namespace esock {


//内部使用
template <class T>
static inline void __on_conn_recvable_helper(net_engine_t *eng,
                                           int sock, 
                                           int event,
                                           T* self)
{
  esock_debug_log("on_conn_recvable\n");

  while (not eng->is_skip_current_event_process()) {
    std::pair<char*, ssize_t> buf_info = self->get_recv_buff();
    char *buf = buf_info.first;
    size_t buflen = buf_info.second;
    size_t recvlen= 0;

    if (buflen == 0) {
      esock_debug_log("recv buffer is full\n");
      return;
    }

    while (recvlen < buflen) {
      ssize_t ret = ::recv(sock, buf+recvlen, buflen-recvlen, 0);
      if (ret == -1) {
        switch (errno) {
        case EINTR:
          continue;

        case EAGAIN:
          if (recvlen)
            self->on_recv_complete(eng, sock, buf, recvlen);
          return;

        default:
          self->on_error_occur(eng, sock, errno);
          return;
        }
      }

      if (ret > 0) {
        recvlen += ret;
        continue;
      }

      // ret == 0
      if (recvlen)
        self->on_recv_complete(eng, sock, buf, recvlen);

      if (not eng->is_skip_current_event_process())
          self->on_peer_close(eng, sock);
      return; //ret == 0
    }

    esock_assert(recvlen == buflen);
    self->on_recv_complete(eng, sock, buf, recvlen);
    esock_debug_log("readbuff full check again\n");
  } 

}


//内部使用
template <class T>
void __post_send(net_engine_t *eng, int sock, T *conn)
{
  //esock_assert(eng->is_in(sock));
  std::pair<const char*, ssize_t> send_data = conn->get_send_data();
  const char *data = send_data.first;
  ssize_t datalen= send_data.second;
  ssize_t sendlen = 0;

  if (datalen == 0)  //使用ET模式无需删除OUT事件
    return;

  while (sendlen < datalen) {
    ssize_t ret = ::send(sock, data + sendlen, datalen -sendlen, MSG_NOSIGNAL);
    if (ret == -1) {
      switch (errno) {
      case EINTR: 
        continue;
      case EAGAIN:
        //esock_debug_log("send fd:%d eagain\n", sock);
        if (sendlen != 0)
          conn->on_send_complete(eng, sock, data, sendlen);
        return;
      default:
        conn->on_error_occur(eng, sock, errno);
        return;
      }
    }

    sendlen += ret;
  }

  if (sendlen != 0)
    conn->on_send_complete(eng, sock, data, sendlen);
}





} //end namespace
