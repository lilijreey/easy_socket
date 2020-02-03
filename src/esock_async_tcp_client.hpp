/**
 *
 */

#pragma once

#include "esock_async_tcp_connection.hpp"

namespace esock
{

/**
 * @brief: Proactor模式的Tcp client接口类.
 *         配合net_engin_t使用提供发起链接和收发数据功能.
 * @usage:
 * <code>
 * EchoClient: async_tcp_client_t<EchoClient> {
 *
 *  //async_tcp_connector_t callbacks
 *  void on_conn_connecting(net_engine_t *eng, int sock){...}
 *  void on_conn_complete(net_engine_t *eng, int sock){...}
 *  void on_conn_failed(net_engine_t *eng, const int err){...}
 *
 *  //async_tcp_connection_t callbacks
 *  void on_recv_complete(net_engine_t *eng, int sock, const char *data, const size_t datalen) {...}
 *  void on_send_complete(net_engine_t *eng, int sock, const char *data, const size_t sendlen) {...}
 *  void on_peer_close(net_engine_t *eng, int sock) {...}
 *  void on_error_occur(net_engine_t *eng, int sock, int error) {...}
 *  std::pair<char*, ssize_t> get_recv_buff() {...}
 *  std::pair<char*, ssize_t> get_send_data() {..}
 *
 * }
 * </code>
 *
 */
template <class subclass_t> 
struct async_tcp_client_t : 
    public async_tcp_connection_t<subclass_t>
{
   /* implements below callback 
     async_tcp_connector_t
     virtual void on_conn_connecting(net_engine_t *eng, int sock);
     virtual void on_conn_complete(net_engine_t *eng, int sock);
     virtual void on_conn_failed(net_engine_t *eng, const int err);

     async_tcp_connection_t
     virtual void on_recv_complete(net_engine_t *eng, int sock, const char *data, const size_t datalen) 
     virtual void on_send_complete(net_engine_t *eng, int sock, const char *data, const size_t sendlen) 
     virtual void on_peer_close(net_engine_t *eng, int sock) 
     virtual void on_error_occur(net_engine_t *eng, int sock, int error) 
     virtual std::pair<char*, ssize_t> get_recv_buff() 
     virtual std::pair<char*, ssize_t> get_send_data()
     */


    //实现async_tcp_connector_t 接口
    static void on_conn_complete_helper (net_engine_t *eng, int sock, void *arg)
    {
      //这时sock 以不在eng中需要重新加入
      esock_assert(not eng->is_add_sock(sock));

      if (-1 == eng->epoll_add_sock (sock,
                                     EPOLLIN,
                                     (void *)subclass_t::on_conn_recvable_helper,
                                     (void *)subclass_t::on_conn_sendable_helper,
                                     arg)) //调用on_tcp_acceptable 成功后设置obj
        {
          //TODO BUG_ON();
          close_socket(sock);
          esock_report_error_msg("new connection %d add engine failed, close it", sock);
          ((subclass_t*)(arg))->on_conn_failed (eng, EINVAL);
          return;
        }

        ((subclass_t*)(arg))->on_conn_complete (eng, sock);
    }

    static void on_conn_failed_helper (net_engine_t *eng, int sock, const int err, void *arg)
    {
        if (err == EINPROGRESS)
          ((subclass_t*)(arg))->on_conn_connecting (eng, sock);
        else
          ((subclass_t*)(arg)) ->on_conn_failed (eng, err);
    }
};


} // end namespace
