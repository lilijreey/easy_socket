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

namespace esock {

/**
 * tcp_server_conn 回调接口, 处理发起链接到读写数据功能.
 * 伪Proactor 模式.
 * usage.
 * <code>
 * EchoServerConn: tcp_server_conn_hanndler_t<EchoServerConn> {
 *  //实现下面几个回调函数
 *
 *  如果在该回调中调用close_socket，则不会进行下面的调用
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
struct tcp_server_conn_handler_t
{

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

/**
 * tcp server,用于监听端口，并初始化连接对象.
 * tcp_server_conn_t 必须是继承tcp_server_conn_handler_t的类型
 * 需要实现
 * usage.
 * <code>
 * EchoServerConn: tcp_server_conn_hanndler_t<EchoServerConn> {
 *  实现下面几个回调函数
 *
 *  返回一个链接实例，如果返回nullptr, 则会自动关闭链接.
 *  如果在on_accept_complete中调用close_socket关闭了链接则必须返回nullptr
 *  返回的连接对象，析构时必须关闭socket,如果需要转移socket和对象的所有关系
 *  可以调用eng->swap_socket(fd, otherConn)
 *  tcp_server_conn_t *on_accept_complete(net_engine_t *eng, tcp_listener_t *ins, int conn, sockaddr_storage addr);
 *
 *  accept 返回失败是调用
 *  void on_accept_failed(net_engine_t *eng, tcp_listener_t *ins, int error);
 * }
 *
 */

template <class subclass_t, class tcp_server_conn_t>
struct tcp_server_handler_t
{
  //tcp_server_conn_t *on_accept_complete(net_engine_t *eng, tcp_listener_t *ins, int conn, sockaddr_storage addr);
  //void on_accept_failed(net_engine_t *eng, tcp_listener_t *ins, int error);

  static void on_tcp_acceptable(net_engine_t *eng, tcp_listener_t *ins, int listen_fd, void *arg)
  {
    sockaddr_storage addr;
    socklen_t addrlen;
    while (true)
    {
      int ret = ::accept4(listen_fd, (sockaddr*)&addr, &addrlen, SOCK_NONBLOCK);
      if (-1 == ret)
      {
        switch (errno) {
        case EAGAIN: return;
        case EINTR: continue;
        default: static_cast<subclass_t*>(arg)->on_accept_failed(eng, ins, errno);
        }
        return;
      }

      esock_debug_log("accept new fd:%d", ret);
      sockinfo_t *sinfo = sockpool.get_info(ret);
      sinfo->init(ESOCKTYPE_TCP_CONNECT);

      if (-1 == eng->epoll_add_sock(ret, EPOLLIN|EPOLLOUT,
                                    (void*)tcp_server_conn_t::on_conn_recvable_helper,
                                    (void*)tcp_server_conn_t::on_conn_sendable_helper,
                                    nullptr)) //调用on_tcp_acceptable 成功后设置obj
      {
        static_cast<subclass_t*>(arg)->on_accept_failed(eng, ins, errno);
        eng->close_socket(ret);
        return;
      }

      tcp_server_conn_t *conn = static_cast<subclass_t*>(arg)->on_accept_complete(eng, ins, ret, addr);
      if (conn == nullptr)
        eng->close_socket(ret);
      else
        sinfo->_arg = conn;
    }
  }

};

} //end namespace
