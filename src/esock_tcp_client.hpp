/**
 * @file     esock_tcp_client.hpp
 *           
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/13/2019 12:17:59 PM
 *
 */

#pragma once

#include "esock_tcp_connect.hpp"

namespace esock {


/**
 * tcp异步链接回调接口 OO风格, 使用CRTP实现编译期重载,避免动态重载性能损耗
 * usage.
 * <code>
 * EchoClient : tcp_connect_hanndler_t<EchoClient> {
 *
 * //连接中,一般用来记录socket,用于取消连接,如果直接链接成功或者其他错误则不会调用该函数
 * void on_conn_connecting(net_engine_t *eng, const char *p, const uint16_t port, int sock)
 * { 
 *   //cancle connect
 *   eng->close_socket(sock);
 * }
 *
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
  //void on_conn_connecting(net_engine_t *eng, const char *p, const uint16_t port, int sock);
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
                                           int sock,
                                           void *arg)
  {
      if (err == EINPROGRESS)
          static_cast<subclass_t*>(arg)->on_conn_connecting(eng, ip, port, sock);
      else
          static_cast<subclass_t*>(arg)->on_conn_failed(eng, ip, port, err);
  }

};

/**
 * tcp_client 回调接口OO模式, 处理发起链接到读写数据功能.
 * 伪Proactor 模式.
 * usage.
 * <code>
 * EchoClient: tcp_client_hanndler_t<EchoClient> {
 *  //实现下面几个回调函数
 *  
 *  void on_conn_connecting(net_engine_t *eng, const char *p, const uint16_t port, int sock)
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
 *  //用来发送数据
 *  post_send();
 *
 * }
 * </code>
 *
 */

template <class subclass_t>
struct async_tcp_client_hanndler_t
{
  /*  请实现下面这些函数
   void on_conn_connecting(net_engine_t *eng, const char *p, const uint16_t port, int sock)

   void on_conn_complete(net_engine_t *eng, const char *ip, const uint16_t port, int sock)
 
   void on_conn_failed(net_engine_t *eng, const char *ip, const uint16_t port, const int err)
 
   void on_recv_complete(net_engine_t *eng, int sock, const char *data, const ssize_t datalen)
 
   void on_send_complete(net_engine_t *eng, int sock, const char *data, const ssize_t sendlen)
 
   void on_peer_close(net_engine_t *eng, int sock)
 
   void on_error_occur(net_engine_t *eng, int sock, int error)
 
   //需要处理buff满的情况
   std::pair<char*, ssize_t> get_recv_buff() 
 
   //返回需要发送的数据
   std::pair<char*, ssize_t> get_send_data()
 */

  static inline void on_conn_complete_helper(net_engine_t *eng,
                                             const char *ip, 
                                             const uint16_t port,
                                             int sock, 
                                             void *arg)
  {

    subclass_t *self = static_cast<subclass_t*>(arg);

    if (sockpool.get_info(sock)->is_closed())
      return;

    if (-1 == eng->epoll_add_sock(sock, EPOLLIN|EPOLLOUT,
                                  (void*)on_conn_recvable_helper,
                                  (void*)on_conn_sendable_helper,
                                  arg))
    {
      self->on_conn_failed(eng, ip, port, errno);
      eng->close_socket(sock);
      return;
    }

    self->on_conn_complete(eng, ip, port, sock);
  }


  static inline void on_conn_failed_helper(net_engine_t *eng,
                                           const char *ip, 
                                           const uint16_t port, 
                                           const int err,
                                           int sock,
                                           void *arg)
  {
      if (err == EINPROGRESS)
          static_cast<subclass_t*>(arg)->on_conn_connecting(eng, ip, port, sock);
      else
          static_cast<subclass_t*>(arg)->on_conn_failed(eng, ip, port, err);
  }

  static inline void on_conn_recvable_helper(net_engine_t *eng,
                                             int sock, 
                                             int event,
                                             void *arg)
  {
    esock_debug_log("on_conn_recvable\n");
    __on_conn_recvable_helper(eng, sock, event, static_cast<subclass_t*>(arg));
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

  //发送数据
  void post_send(net_engine_t *eng, int sock)
  {
    __post_send(eng, sock, static_cast<subclass_t*>(this));
  }


};


} //end namespace
