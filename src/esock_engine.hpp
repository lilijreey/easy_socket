/**
 * @file     esock_engine.hpp
 *           
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/05/2019 01:31:52 PM
 *
 */

#pragma once

#include <string>
#include <chrono>

#include <sys/epoll.h>

#include "esock_socket.hpp"
#include "esock_error.hpp"
#include "detail/esock_utility.hpp"
#include "esock_tcp_listener.hpp"

namespace esock {

class net_engine_t;
class tcp_listener_t;
struct sockinfo_t;

/**
 * 回调类型
 */
//tcp_client cb
using on_tcp_conn_complete_fn_t = void(*)(net_engine_t *eng, const char *ip, const uint16_t port, int sock, void *arg);
using on_tcp_conn_failed_fn_t   = void(*)(net_engine_t *eng, const char *ip, const uint16_t port, const int err, int sock, void *arg);

//tcp_server cb
using on_tcp_listener_acceptable_fn_t = void(*)(net_engine_t *eng, tcp_listener_t *ins, int listen_fd, void *arg);
//using on_tcp_accept_complete_fn_t = void(*)(net_engine_t *eng, tcp_listener_t *svr, int sock, void *user_arg);
//using on_tcp_accept_failed_fn_t   = void(*)(net_engine_t *eng, tcp_listener_t *svr, int errno, void *user_arg);

//tcp connect cb
using on_tcp_conn_recvable_fn_t = void(*)(net_engine_t *eng, int sock, int _event, void *arg);
using on_tcp_conn_sendable_fn_t = void(*)(net_engine_t *eng, int sock, int _event, void *arg);


enum sockevent_t
{
  ESOCK_EV_RECVABLE = 1,
  ESOCK_EV_SENDABLE = 2,
  ESOCK_EV_ERROR = 4,
};


class net_engine_t : detail::noncopyable_t
{
 public:
  static net_engine_t * make();
  static void unmake(net_engine_t *&eng);

 public:
  ~net_engine_t();

 public:
  int close_socket(int sock);

 public: //tcp connect
  //发起异步链接请求
  //面向OO T 类型必须继承tcp_connect_hanndler_t
  //BUG TODO 链接过程中对象销毁
  template <class async_tcp_connect_hanndler_subclass>
  void async_tcp_connect(const std::string &ip, 
                         const uint16_t port, 
                         async_tcp_connect_hanndler_subclass *ins);

 public:
  //tcp client
  //比async_tcp_connect 更高层的接口
  //发起异步链接请求，当链接完成是把链接加入engine,管理并自动读取，写入数据
  //类Proactor 模式.
  template <class async_tcp_client_hanndler_subclass>
  void async_tcp_client(const std::string &ip,
                        const uint16_t port,
                        async_tcp_client_hanndler_subclass *ins);


 public:
  int add_tcp_listener(tcp_listener_t *listener,
                       on_tcp_listener_acceptable_fn_t on_acceptable_fn,
                       void *arg);
  //int del_tcp_listener //TODO

  
 public:  //tcp server
  template <class tcp_server_handler_subclass>
  int add_tcp_server(tcp_listener_t *listener, tcp_server_handler_subclass *ins);



 public:
  bool is_runing() const {return not _is_stop;}
  bool is_stop() const {return _is_stop;}


 public: //process event
  int process_event(std::chrono::milliseconds wait_event_ms);

  int process_event_loop(std::chrono::milliseconds wait_event_ms)
  {
    while (is_runing()) {
      if (-1 == process_event(wait_event_ms))
        return -1;
    }
    return 0;
  }

  void stop_event_loop() {_is_stop = true;}


 private:
  net_engine_t() = default;
  int init();

 private:
  void async_tcp_connect(const std::string &ip, 
                        const uint16_t port, 
                        on_tcp_conn_complete_fn_t, 
                        on_tcp_conn_failed_fn_t,
                        void *user_arg=nullptr);

 public:
  //无法重复添加
  //保证对应sockslot 已经设置正确
  //当错误发送时调用readable 来发现错误,而不提供错误函数
  int epoll_add_sock(int fd,
                     int events, 
                     void *on_recvable,
                     void *on_sendable,
                     void *arg);

  //void epoll_del_sock(int fd);
  void epoll_del_sock(sockinfo_t *sinfo);


 private:
  bool _is_stop = false;

  //epoll
 private:
  enum {MAX_EVENT_SIZE=1024};

 private:
  int _efd = -1;
  bool _is_process= false;
  epoll_event _events[MAX_EVENT_SIZE];

};


static inline net_engine_t* make_net_engine() { return net_engine_t::make(); }
static inline void unmake_net_engine(net_engine_t *&eng) { net_engine_t::unmake(eng); }

} //end namespace


#include "esock_tcp_client.hpp"
#include "esock_tcp_server.hpp"

namespace esock {

template <class async_tcp_connect_hanndler_subclass>
void net_engine_t::async_tcp_connect(const std::string &ip, 
                       const uint16_t port, 
                       async_tcp_connect_hanndler_subclass *ins)
{
  using T = async_tcp_connect_hanndler_subclass;
  async_tcp_connect(ip,
                    port,
                    T::on_conn_complete_helper,
                    T::on_conn_failed_helper,
                    ins);
}


//发起异步链接请求，当链接完成是把链接加入engine,管理并自动读取，写入数据
//类Proactor 模式.

template <class async_tcp_client_hanndler_subclass>
void net_engine_t::async_tcp_client(const std::string &ip,
                      const uint16_t port,
                      async_tcp_client_hanndler_subclass *ins)
{
  using T = async_tcp_client_hanndler_subclass;
  async_tcp_connect(ip,
                    port,
                    T::on_conn_complete_helper,
                    T::on_conn_failed_helper,
                    ins);
}


template <class tcp_server_handler_subclass_t>
int net_engine_t::add_tcp_server(tcp_listener_t *listener, tcp_server_handler_subclass_t *ins)
{
  using T = tcp_server_handler_subclass_t;
  return add_tcp_listener(listener, T::on_tcp_acceptable, ins);
}


} //end namespace
