
#pragma once

#include "detail/esock_sysinclude_fs.hpp"
#include "detail/esock_error.hpp"
#include "esock_engine.hpp"

namespace esock {


//链接读写接口用于async_tcp_client_t async_tcp_server
template<class subclass>
struct async_tcp_connection_t
{
  /* subclass needs implement theos callbacks 
     void on_recv_complete(net_engine_t *eng, int sock, const char *data, const size_t datalen) =0;
     void on_send_complete(net_engine_t *eng, int sock, const char *data, const size_t sendlen) =0;
     void on_peer_close(net_engine_t *eng, int sock) = 0;
     void on_error_occur(net_engine_t *eng, int sock, int error) = 0;
     //需要处理buff满的情况
     std::pair<char*, ssize_t> get_recv_buff() = 0;
     //返回需要发送的数据
     std::pair<char*, ssize_t> get_send_data() = 0;
  */

    //调用该函数发送数据 
    //void post_send (net_engine_t *eng, int sock);
    //void post_send (int sock);

    static void on_conn_sendable_helper (net_engine_t *eng, int sock, int event, void *arg)
    {
        //发送待发送数据
       esock_debug_log ("fd:%d on_sendable\n", sock);
       static_cast<async_tcp_connection_t*>(arg)->post_send (eng, sock);
    }


    //发送数据
    void post_send (net_engine_t *eng, int sock)
    {
      // esock_assert(eng->is_in(sock));
      std::pair<const char *, ssize_t> send_data = static_cast<subclass*>(this)->get_send_data ();
      const char *data = send_data.first;
      ssize_t datalen = send_data.second;
      ssize_t sendlen = 0;

      if (datalen == 0)
      {
        if (eng->is_add_sendable_ev (sock))
        {
          if (-1 == eng->del_sendable_ev (sock)) 
            static_cast<subclass*>(this)->on_error_occur (eng, sock, errno);
        }
        return;
      }

      while (sendlen < datalen)
      {
        ssize_t ret = ::send (sock, data + sendlen, datalen - sendlen, MSG_NOSIGNAL);
        if (ret == -1)
        {
          switch (errno)
          {
          case EINTR:
            continue;
          case EAGAIN:
            if (-1 == eng->add_sendable_ev (sock))
            {
             static_cast<subclass*>(this)-> on_error_occur (eng, sock, errno);
              return;
            }
            goto out;
          default:
            static_cast<subclass*>(this)-> on_error_occur (eng, sock, errno);
            return;
          }
        }

        sendlen += ret;
      }

out:
      if (sendlen != 0) 
        static_cast<subclass*>(this)->on_send_complete (eng, sock, data, sendlen);
    }

    //调用该函数发送数据
    void post_send (int sock)
    {
       post_send(sockpool.get_info (sock)->_eng, sock);
    }


    static void on_conn_recvable_helper (net_engine_t *eng, int sock, int event, void *arg)
    {
      esock_debug_log ("on_conn_recvable\n");
      subclass* self = (subclass*)(arg);

      while (not eng->is_skip_current_event_process ())
      {
        std::pair<char *, ssize_t> buf_info = self->get_recv_buff ();
        char *buf = buf_info.first;
        size_t buflen = buf_info.second;
        size_t recvlen = 0;

        //TODO full 精细处理
        if (buflen == 0) {
            esock_debug_log ("recv buffer is full");
            self->on_recv_complete (eng, sock, buf, 0); //通知上层处理数据
            return;
        }


        while (recvlen < buflen)
        {
          ssize_t ret = ::recv (sock, buf + recvlen, buflen - recvlen, 0);
          if (ret == -1)
          {
            switch (errno)
            {
            case EINTR:
              continue;
            case EAGAIN:
              if (recvlen) 
                self->on_recv_complete (eng, sock, buf, recvlen);
              return;
            default:
              self->on_error_occur (eng, sock, errno);
              return;
            }
          }

          if (ret > 0)
          {
            recvlen += ret;
            continue;
          }

          // ret == 0
          if (recvlen) 
            self->on_recv_complete (eng, sock, buf, recvlen);

          if (not eng->is_skip_current_event_process ()) 
            self->on_peer_close (eng, sock);
          return;
        }

        esock_assert (recvlen == buflen);
        self->on_recv_complete (eng, sock, buf, recvlen);
        esock_debug_log("readbuff full check again\n");
      }
    }
};



} //end namespace
