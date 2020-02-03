/**
 * @file     esock_async_tcp_server.hpp
 *
 */

#pragma once

#include "esock_async_tcp_connection.hpp"

namespace esock
{

class net_engine_t;
class tcp_listener_t;

/**
 * @brief: 提供tcp server基础能力于监听端口，并初始化连接对象.
 * tcp_server_conn_t 必须是继承tcp_server_conn_handler_t的类型
 * 需要实现
 * usage.
 * <code>
 * EchoServerConn : async_tcp_connection_t {...}
 *
 * EchoServer: async_tcp_server_t<EchoServerConn> {
 *  需要实现下面几个回调函数
 *
 *  1. tcp_server_conn_t *on_accept_complete(net_engine_t *eng, tcp_listener_t
 *                                           *ins, int conn, sockaddr_storage addr);
 *  返回一个链接实例，如果返回nullptr, 则会自动关闭链接.
 *  如果在on_accept_complete中调用close_socket关闭了链接则必须返回nullptr
 *  返回的连接对象，析构时必须关闭socket,如果需要转移socket和对象的所有关系
 *  可以调用eng->swap_socket(fd, otherConn)
 *
 *  2. void on_accept_failed(net_engine_t *eng, tcp_listener_t *ins, int error);
 *     accept 返回失败是调用
 * }
 *
 */

template <class subclass_t, class tcp_server_conn_t> 
struct async_tcp_server_t 
{
  static_assert(std::is_base_of<async_tcp_connection_t<tcp_server_conn_t>, tcp_server_conn_t>::value,
                "Bad type server_conntion_t must is an async_tcp_connection_t");

    // tcp_server_conn_t *on_accept_complete(net_engine_t *eng, tcp_listener_t *ins, int conn, sockaddr_storage addr);
    // void on_accept_failed(net_engine_t *eng, tcp_listener_t *ins, int error);

    static void on_tcp_acceptable (net_engine_t *eng, tcp_listener_t *ins, int listen_fd, void *arg)
    {
        sockaddr_storage addr;
        socklen_t addrlen = sizeof (addr);
        while (true)
        {
            int ret = ::accept4 (listen_fd, (sockaddr *)&addr, &addrlen, SOCK_NONBLOCK);
            if (-1 == ret)
            {
                switch (errno)
                {
                case EAGAIN:
                    return;
                case EINTR:
                    continue;
                default:
                    static_cast<subclass_t *> (arg)->on_accept_failed (eng, ins, errno);
                }
                return;
            }

            esock_debug_log ("accept new fd:%d", ret);
            sockinfo_t *sinfo = sockpool.get_info (ret);
            sinfo->init (ESOCKTYPE_TCP_CONNECT);

            if (-1 ==
                eng->epoll_add_sock (ret, 
                                     EPOLLIN,
                                     (void *)tcp_server_conn_t::on_conn_recvable_helper,
                                     (void *)tcp_server_conn_t::on_conn_sendable_helper,
                                     nullptr)) //调用on_tcp_acceptable 成功后设置obj
            {
                static_cast<subclass_t *> (arg)->on_accept_failed (eng, ins, errno);
                eng->close_socket (ret);
                return;
            }

            tcp_server_conn_t *conn =
            static_cast<subclass_t *> (arg)->on_accept_complete (eng, ins, ret, addr);
            if (conn == nullptr)
                eng->close_socket (ret);
            else
                sinfo->_arg = conn;
        }
    }
};


} // end namespace
