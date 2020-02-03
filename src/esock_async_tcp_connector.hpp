#pragma once


#include "detail/esock_sysinclude_fs.hpp"

namespace esock
{

class net_engine_t;
/**
 * @brief: tcp 异步conncet 回调接口,完成tcp链接功能.
 *        一般不直接使用.
 *
 * @usage:
 * <code>
 * EchoTcpClient : esock::async_tcp_connector_t<EchoClient> {
 * //需要实现如下回调函数
 *
 * //connect 返回EPROGRESS  时回调
 * void on_conn_connecting(net_engine_t *eng, int sock);
 *
 * //connect 返回成功调用，或EPROGRESS 链接成功后调用
 * //如果返回false 会自动关闭sock,一般在回调出现错误时使用
 * bool on_conn_complete(net_engine_t *eng, int sock);
 *
 * //connect 失败后,或者EPROGRESS失败后调用
 * void on_conn_failed(net_engine_t *eng, const int err);
 *
 * @usage:
 *   see examples/tcp
 *
 */

template <class subclass_t> 
struct async_tcp_connector_t
{
    static void on_conn_complete_helper (net_engine_t *eng, int sock, void *arg)
    {
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


} //end namespace esock
