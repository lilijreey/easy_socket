/**
 * @file     esock_async_tcp_conn_handler.hpp
 *           
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/05/2019 04:54:14 PM
 *
 */

#pragma once


//#include "esock_engine.hpp"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "detail/esock_sockpool.hpp"

namespace esock {

//class net_engine_t;



/**
 * tcp异步链接回调接口 OO风格, 使用CRTP实现编译期重载,避免动态重载性能损耗
 * usage.
 * <code>
 * EchoClient : tcp_connect_hanndler_t<EchoClient> {
 *
 *  void on_conn_complete(net_engine_t *eng, const char *ip, const uint16_t port, int sock) {
 *    send(sock, "hello", 6);
 *    //... code
 *  }
 *  void on_conn_failed(net_engine_t *eng, const char *ip, const uint16_t port, const int err) {
 *    //... code
 *  }
 * }
 * </code>
 *
 */

template <class subclass_t>
struct async_tcp_connect_hanndler_t
{
  //subclass_t need implete these two func
  //void on_conn_complete(net_engine_t *eng, const char *ip, const uint16_t port, int sock);
  //void on_conn_failed(net_engine_t *eng, const char *ip, const uint16_t port, const int err);

  static inline void on_conn_complete_helper(net_engine_t *eng,
                                             const char *ip, 
                                             const uint16_t port,
                                             int sock, 
                                             void *arg)
  {

    static_cast<subclass_t*>(arg)->on_conn_complete(eng, ip, port, sock);
  }

  static inline void on_conn_failed_helper(net_engine_t *eng,
                                           const char *ip, 
                                           const uint16_t port, 
                                           const int err,
                                           void *arg)
  {
    static_cast<subclass_t*>(arg)->on_conn_failed(eng, ip, port, err);
  }

};

/**
 * tcp_client 回调接口OO模式, 处理发起链接到读写数据功能.
 * 伪Proactor 模式.
 * usage.
 * <code>
 * EchoClient: async_tcp_client_hanndler_t<EchoClient> {
 *  //实现下面几个回调函数
 *
 *   如果在该回调中调用close_socket，则不会进行下面的调用
 *  void on_conn_complete(net_engine_t *eng, const char *ip, const uint16_t port, int sock);
 *
 *  void on_conn_failed(net_engine_t *eng, const char *ip, const uint16_t port, const int err);
 *
 *  void on_recv_complete(net_engine_t *eng, int sock, const char *data, const ssize_t datalen);
 *
 *   发送完成数据是通知,一般用来更新buff
 *  void on_send_complete(net_engine_t *eng, int sock, const char *data, const ssize_t sendlen);
 *
 *  void on_peer_close(net_engine_t *eng, int sock);
 *
 *  void on_error_occur(net_engine_t *eng, int sock, int error);
 *
 *  //需要处理buff满的情况
 *  std::pair<char*, ssize_t> get_recv_buff() {...}
 *
 *  //返回需要发送的数据
 *  std::pair<char*, ssize_t> get_send_data() {..}
 *
 *  post_send();
 *
 * }
 * </code>
 *
 */

template <class subclass_t>
struct async_tcp_client_hanndler_t
{

  //发送数据
  void post_send(net_engine_t *eng, int sock)
  {
      std::pair<const char*, ssize_t> send_data = static_cast<subclass_t*>(this)->get_send_data();
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
              static_cast<subclass_t*>(this)->on_send_complete(eng, sock, data, sendlen);
            return;
          default:
            static_cast<subclass_t*>(this)->on_error_occur(eng, sock, errno);
            return;
          }
        }

        sendlen += ret;
      }

      if (sendlen != 0)
        static_cast<subclass_t*>(this)->on_send_complete(eng, sock, data, sendlen);
  }

  static inline void on_conn_failed_helper(net_engine_t *eng,
                                           const char *ip, 
                                           const uint16_t port, 
                                           const int err,
                                           void *arg)
  {
    static_cast<subclass_t*>(arg)->on_conn_failed(eng, ip, port, err);
  }

  static inline void on_conn_recvable_helper(net_engine_t *eng,
                                             int sock, 
                                             int event,
                                             void *arg)
  {
    esock_debug_log("on_conn_recvable\n");
    subclass_t *self = static_cast<subclass_t*>(arg);

    while (true) {
      //get_bufer_info;
      std::pair<char*, ssize_t> buf_info = self->get_buffer_info();
      char *buf = buf_info.first;
      ssize_t buflen = buf_info.second;
      ssize_t recvlen= 0;

      if (buflen == 0) {
        esock_debug_log("recv buffer is full");
        return;
      }

      while (recvlen < buflen) {
        ssize_t ret = ::recv(sock, buf+recvlen, buflen-recvlen, 0);
        if (ret == -1) {
          if (errno == EINTR) 
            continue;

          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            if (recvlen)
              self->on_recv_complete(eng, sock, buf, recvlen);
            return;
          }

          self->on_error_occur(eng, sock, errno);
          return;
        }

        if (ret > 0) {
          recvlen += ret;
          continue;
        }

        // ret == 0
        if (recvlen)
          self->on_recv_complete(eng, sock, buf, recvlen);

        self->on_peer_close(eng, sock);
        return;
      }

      esock_assert(recvlen == buflen);
      self->on_recv_complete(eng, sock, buf, recvlen);
    } 

  }

  static inline void on_conn_sendable_helper(net_engine_t *eng,
                                             int sock, 
                                             int event,
                                             void *arg)
  
  {
    //发送待发送数据
    subclass_t *self = static_cast<subclass_t*>(arg);
    esock_debug_log("fd:%d on_sendable\n", sock);
    self->post_send(eng, sock);
  }

  static inline void on_conn_complete_helper(net_engine_t *eng,
                                             const char *ip, 
                                             const uint16_t port,
                                             int sock, 
                                             void *arg)
  {

    subclass_t *self = static_cast<subclass_t*>(arg);
    self->on_conn_complete(eng, ip, port, sock);

    if (sockpool.get_info(sock)->is_closed())
      return;

    if (-1 == eng->epoll_add_sock(sock, EPOLLIN|EPOLLOUT,
                                  (void*)on_conn_recvable_helper,
                                  (void*)on_conn_sendable_helper,
                                  arg))
    {
      self->on_error_occur(eng, sock, errno);
    }

  }


};




} //end namespace
