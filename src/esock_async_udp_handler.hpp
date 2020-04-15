#pragma once

#include "detail/esock_sysinclude_fs.hpp"
#include "detail/esock_error.hpp"
#include "esock_engine.hpp"

namespace esock {

/* 子类需要实现如下函数
 *
 *   void on_recv_complete(net_engine_t *eng, int sock, const char *data, const size_t datalen, sockaddr_storage *);
 *   void on_error_occur(net_engine_t *eng, int sock, int error) = 0;
 *   //需要处理buff满的情况
 *   std::pair<char*, ssize_t> get_recv_buff() = 0;
 */
template <class subclass_t>
struct async_udp_handler_t
{
 public:
  static void on_conn_recvable_helper(net_engine_t *eng, int sock, int err, void *arg)
  {
    esock_debug_log ("udp_on_conn_recvable\n");
    subclass_t* self = (subclass_t*)(arg);
    sockaddr_storage addr;
    socklen_t addrlen = sizeof (addr);

    //TODO socket公平性是否会一直处理一个socket
    while (not eng->is_skip_current_event_process ())
    {
      std::pair<char *, ssize_t> buf_info = self->get_recv_buff ();
      char *buf = buf_info.first;
      size_t buflen = buf_info.second;

      //TODO full 精细处理
      if (buflen == 0) {
        esock_debug_log ("recv buffer is full");
        self->on_recv_complete (eng, sock, buf, 0, NULL, 0); //通知上层处理数据
        return;
      }

      ssize_t ret = ::recvfrom (sock, buf, buflen, 0, (sockaddr *)&addr, &addrlen);
      if (ret > 0) {
        self->on_recv_complete (eng, sock, buf, ret, &addr, addrlen);
        continue;
      } 

      //ret == -1
      switch (errno)
      {
      case EINTR:
        continue;
      case EAGAIN:
        return;
      default:
        self->on_error_occur (eng, sock, errno);
        return;
      } 
    } //end while
  }

  static void on_conn_sendable_helper(net_engine_t *eng, int sock, int err, void *arg)
  {
    esock_debug_log("can not watch EPOLLOUT in udp socket");
    //不处理该情况
  }

  static constexpr bool is_base_of_async_udp_handler_t = true;

};


//template <class T>
//struct is_base_of_async_udp_handler {
//  static constexpr bool value = typename T::is_base_of_async_udp_handler_t;
//};


} //end namespace esock
