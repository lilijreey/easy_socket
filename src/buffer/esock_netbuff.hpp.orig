/**
 * @file     package_buffer.hpp
 *
 *
 * @brief  在IO层上实现一种简单的buff管理策略+解包策略,提供给上层业务侧使用.
 *
 *
 */

#pragma once

#include "../detail/esock_error.hpp"
#include "esock_raw_buff.hpp"
#include "esock_ring_buff.hpp"

namespace esock {

#define self static_cast<subclass_t *>(this)

template <class subclass_t>
struct net_fixbuff_op_t
{
    //返回能给个写入的长度
    std::pair<char *, size_t> get_recv_buff ()
    {
        self->_recvbuf.discard_readed_data ();

        //如果buffsize 大于最大单个包长则不存full的情况

        return { self->_recvbuf.get_writable_buff (), self->_recvbuf.get_writable_len () };
    }

    void on_recv_complete (net_engine_t *eng, int sock, const char *data, const size_t datalen)
    {
        //接受的数据写入get_recv_buff 返回的空间,
        // dataLen有可能是0应用层buff已满无法接受,但是还是要通知应用层消费他的数据

        assert (self->_recvbuf.get_writable_len() <= datalen);
        //self->_recvbuf.increase_write_len(datalen);

        while (const typename subclass_t::pkg_head_t *head = self->_recvbuf.peek_package_head())
        {
            if (not head->is_valid_head ())
            {
                static_cast<subclass_t *> (this)->on_recv_pkg_invalid (eng, sock, head);
                return;
            }

            if (not self->_recvbuf.is_complete_pkg (head->get_pkg_len ()))
            {
                if (self->_recvbuf.is_full ())
                { //出现这种情况时代表有BUG发生
                    esock_report_error_msg (
                    "sock %d buff is full but pkg not complete should not occu this", sock);
                    static_cast<subclass_t *> (this)->on_recv_pkg_invalid (eng, sock, head);
                }
                return;
            }

            self->_recvbuf.read_package (head->get_pkg_len ());

            static_cast<subclass_t *> (this)->on_recv_pkg_complete (eng, sock, head,
                                                                    head->get_pkg_len () - sizeof (typename subclass_t::pkg_head_t));

            if (eng->is_skip_current_event_process ()) return;
        }
    }

    //返回需要发送的数据
    //自动实现
    std::pair<const char *, ssize_t> get_send_data ()
    {
<<<<<<< HEAD
        return { self->_sendbuf.get_sendable_data (), self->_sendbuf.get_sendable_len () };
=======
      _wpos += len;
      assert(_wpos <= BUFSIZE);
    }

    void update_recved_data(size_t datalen)
    {
        increase_write_len(datalen);
>>>>>>> 0cc55b20ffef6473b8410c1899d2558dabfe174c
    }

    //自动实现
    void on_send_complete (net_engine_t *eng, int sock, const char *data, const ssize_t sendlen)
    {
        self->_sendbuf.update_sended_data (sendlen);

        self->_sendbuf.discard_sended_data (); //TODO 合并到update_sended_data 中

        if (self->_sendbuf.get_sendable_len () != 0)
            self->post_send (eng, sock);
    }

};
#undef self

//TODO 改造buffer大小运行时可跳,允许配置
template <typename pkg_head_t, size_t RECVBUF_SIZE, size_t SENDBUF_SIZE=RECVBUF_SIZE, size_t PKG_HEAD_LEN=sizeof(pkg_head_t)>
struct net_rawbuf_t
    :net_fixbuff_op_t<net_rawbuf_t>
{
    protected:
    net_recv_rawbuf_t<pkg_head_t, RECVBUF_SIZE> _recvbuf;
    net_send_rawbuf_t<pkg_head_t, SENDBUF_SIZE> _sendbuf;
};

<<<<<<< HEAD
=======
   //返回能给个写入的长度
   std::pair<char*, size_t> get_recv_buff() 
   {
       _recvbuf.discard_readed_data();

       //如果buffsize 大于最大单个包长则不存full的情况

       return {
           _recvbuf.get_writable_buff(),
           _recvbuf.get_writable_len()
       };
   }

   void on_recv_complete(net_engine_t *eng, int sock, const char *data, const size_t datalen)
   {
       //接受的数据写入get_recv_buff 返回的空间,
       assert(datalen != 0);
       _recvbuf.update_recved_data(datalen);

        while (const pkg_head_t *head = _recvbuf.peek_package_head())
        {
            if (not head->is_valid_head())
            {
                //TODO 只检查一次
                static_cast<subclass_t*>(this)->on_recv_pkg_invalid(eng, sock, head);
                return;
            }

            if (not _recvbuf.is_complete_pkg(head->get_pkg_len()))
                return;

            _recvbuf.read_package(head->get_pkg_len()); //XXX 省略掉参数

            static_cast<subclass_t*>(this)->on_recv_pkg_complete(eng, sock, head, head->get_pkg_len()- sizeof(pkg_head_t));

            if (eng->is_skip_current_event_process())
                return;
        }
   }

   //返回需要发送的数据
   //自动实现
   std::pair<const char*, ssize_t> get_send_data()
   {
     return {
       _sendbuf.get_sendable_data(), _sendbuf.get_sendable_len()
     };
   }
 
   //自动实现
   void on_send_complete(net_engine_t *eng, int sock, const char *data, const ssize_t sendlen)
   {
     _sendbuf.update_sended_data(sendlen);

    //已发送数据超过buf长度的1/4后调整
    if (_sendbuf._rpos>= (SENDBUF_SIZE>>2))
        _sendbuf.discard_sended_data();

    if (_sendbuf.get_sendable_len() != 0)
        static_cast<subclass_t*>(this)->post_send(eng, sock);
   }
>>>>>>> 0cc55b20ffef6473b8410c1899d2558dabfe174c

//template <typename pkg_head_t, size_t BUFSIZE, size_t PKG_HEAD_LEN=sizeof(pkg_head_t)>
//struct net_ringbuff_t
//    :net_fixbuff_op_t<net_raw_fixbuff_t>
//{
//    protected:
//    net_recv_ringbuf_t<pkg_head_t, RECVBUF_SIZE> _recvbuf;
//    net_send_ringbuf_t<pkg_head_t, SENDBUF_SIZE> _sendbuf;
//};

//TODO type builder

/**
 * 组合IO层 + 收发buffer + 解包功能.
 * usage:
 * <code>
 * class BinClient : public tcp_active_conn_with_rawbuff_t<BinClient, pkg_head_t, _4K>  {
 * //需要实现如需函数
 * //connect callback
 * void on_conn_connecting(net_engine_t *eng,  int sock)
 * void on_conn_complete(net_engine_t *eng, int sock)
 * void on_conn_failed(net_engine_t *eng, const int err)
 *
 *
 * //当发现一个无效包时回调，应该关闭sock,如果不关闭行为未定义
 * void on_recv_pkg_invalid(net_engine_t *eng, int sock, const pkg_head_t *head)
 * void on_recv_pkg_complete(net_engine_t *eng, int sock, const pkg_head_t *pkg,
 * const size_t bodylen)
 *
 * void on_peer_close(net_engine_t *eng, int sock)
 * void on_error_occur(net_engine_t *eng, int sock, int error)
 *
 * }
 * </code>
 */

template <class subclass_t, class pkg_head_t, size_t RECVBUF_SIZE, size_t SENDBUF_SIZE = RECVBUF_SIZE>
struct tcp_active_conn_with_raw_buff_t 
  : async_tcp_client_hanndler_t<subclass_t>
  , net_rawbuff_t<subclass_t, pkg_head_t, RECVBUF_SIZE, SENDBUF_SIZE>
{};

template <class subclass_t, class pkg_head_t, size_t RECVBUF_SIZE, size_t SENDBUF_SIZE = RECVBUF_SIZE>
struct tcp_active_conn_with_ring_buff_t 
: public async_tcp_client_hanndler_t<subclass_t>
, public net_ringbuff_t<subclass_t, pkg_head_t, RECVBUF_SIZE, SENDBUF_SIZE>
{};

//you can using an alias

/**
 * 组合IO层 + 收发buffer + 解包功能,方便业务层使用.
 * usage:
 * <code>
 * class BinServerConn: public tcp_passive_conn_t<BinServerConn, pkg_head_t,
 * _4K>  {
 * //需要实现如需函数
 * //connect callback
 *
 * //当发现一个无效包时回调，应该关闭sock,如果不关闭行为未定义
 * void on_recv_pkg_invalid(net_engine_t *eng, int sock, const pkg_head_t *head)
 * void on_recv_pkg_complete(net_engine_t *eng, int sock, const pkg_head_t *pkg,
 * const size_t bodylen)
 *
 * void on_peer_close(net_engine_t *eng, int sock)
 * void on_error_occur(net_engine_t *eng, int sock, int error)
 *
 *  pkg_head_t 类型需要实现下面函数
 *  bool is_valid_head() const 
 *  uint32_t get_pkg_len() const 
 *
 * }
 * </code>
 */
template <class subclass_t, class pkg_head_t, size_t RECVBUF_SIZE, size_t SENDBUF_SIZE = RECVBUF_SIZE>
struct tcp_passive_conn_t : public tcp_server_conn_handler_t<subclass_t>,
                            public net_fixbuff_t<subclass_t, pkg_head_t, RECVBUF_SIZE, SENDBUF_SIZE>
{
};

} // end namespace
