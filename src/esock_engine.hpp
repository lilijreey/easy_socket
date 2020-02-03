/**
 * @file     esock_engine.hpp
 *
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/05/2019 01:31:52 PM
 *
 */

#pragma once

#include "detail/esock_error.hpp"
#include "detail/esock_utility.hpp"
#include "detail/esock_sockpool.hpp"
#include "esock_socket.hpp"


namespace esock
{

class tcp_listener_t;
struct sockinfo_t;


int close_socket (int fd);


enum sockevent_t
{
    ESOCK_EV_RECVABLE = 1,
    ESOCK_EV_SENDABLE = 2,
    ESOCK_EV_ERROR = 4,
};

class net_engine_t : detail::noncopyable_t
{
 public:
  //回调接口
  // tcp_client cb
  using on_tcp_conn_complete_fn_t = void (*) (net_engine_t *eng, int sock, void *arg);
  using on_tcp_conn_failed_fn_t = void (*) (net_engine_t *eng, int sock, const int err, void *arg);

  // tcp_server cb
  using on_tcp_listener_acceptable_fn_t = void (*) (net_engine_t *eng, tcp_listener_t *ins, int listen_fd, void *arg);

  // tcp connect cb
  using on_tcp_conn_recvable_fn_t = void (*) (net_engine_t *eng, int sock, int _event, void *arg);
  using on_tcp_conn_sendable_fn_t = void (*) (net_engine_t *eng, int sock, int _event, void *arg);

  // udp connect cb
  using on_udp_conn_recvable_fn_t = void (*) (net_engine_t *eng, int sock, int _event, void *arg);
  using on_udp_conn_sendable_fn_t = void (*) (net_engine_t *eng, int sock, int _event, void *arg);

 public:
  static net_engine_t *make (void);

  /**
   * @brief: Destroy net_engine_t instance return by make
   * and do not auto close sockets join in this eng.
   * you should keep all socket have been closed before call this;
   * @param eng [in,out] will be set to nullptr;
   */
  static void unmake (net_engine_t *&eng);

 public:
  /**
   * see unmake()
   */
  ~net_engine_t ();

 public:
  int close_socket (int sock);

 public: // tcp connect
  //发起异步链接请求
  //面向OO T 类型必须继承tcp_connect_hanndler_t
  template <class async_tcp_connector_subclass>
  void add_async_tcp_connector(const std::string &ip, const uint16_t port, async_tcp_connector_subclass *ins)
  {
    async_raw_tcp_connect (ip, 
                           port, 
                           async_tcp_connector_subclass::on_conn_complete_helper, 
                           async_tcp_connector_subclass::on_conn_failed_helper, 
                           ins);
  }

 public:
  // tcp server 
  int add_raw_tcp_server(tcp_listener_t *listener, on_tcp_listener_acceptable_fn_t on_acceptable_fn, void *arg);

 public:
  // tcp client
  //比async_tcp_connect 更高层的接口
  //发起异步链接请求，当链接完成是把链接加入engine,管理并自动读取，写入数据
  //类Proactor 模式.
  template <class async_tcp_client_subclass>
  void add_async_tcp_client (const std::string &ip, const uint16_t port, async_tcp_client_subclass *ins)
  {
    async_raw_tcp_connect(ip, 
                          port, 
                          async_tcp_client_subclass::on_conn_complete_helper,
                          async_tcp_client_subclass::on_conn_failed_helper, 
                          ins);
  }


  // tcp server
  template<class async_tcp_server_subclass>
  int add_async_tcp_server (tcp_listener_t *listener, async_tcp_server_subclass *ins)
  {
    return add_raw_tcp_server(listener, 
                              async_tcp_server_subclass::on_tcp_acceptable,
                              ins);
  }


 public:
  //TODO 非阻塞UDP会出现EBLOCK吗
  template <class async_udp_client_subclass >
  int add_async_udp_client (const std::string &dst_ip, const uint16_t dst_port, async_udp_client_subclass *ins)
  {
    int sock = make_udp_client_socket(dst_ip, dst_port);
    if (-1 == epoll_add_sock (sock,
                              EPOLLIN,
                              (void *)async_udp_client_subclass::on_conn_recvable_helper,
                              (void *)async_udp_client_subclass::on_conn_sendable_helper,
                              ins)) 
    {
      //ON_BUG
      close_socket (sock);
      esock_report_error_msg("add udp socket %d to eng failed", sock);
      return -1;
    } 

    return sock;
  }


  template <class async_udp_connection_subclass>
  int add_async_udp_server(const std::string &src_ip, const uint16_t src_port, async_udp_connection_subclass *ins)
  {
    int sock = make_udp_server_socket(src_ip, src_port);
    if (-1 == epoll_add_sock (sock,
                              EPOLLIN,
                              (void *)async_udp_connection_subclass::on_conn_recvable_helper,
                              (void *)async_udp_connection_subclass::on_conn_sendable_helper,
                              ins)) 
    {
      //ON_BUG
      close_socket (sock);
      esock_report_error_msg("add udp socket %d to eng failed", sock);
      return -1;
    } 

    return sock;
  }


 public:
  bool is_runing () const { return not _is_stop; }
  bool is_stop () const { return _is_stop; }

 public: // process event
  int process_event (std::chrono::milliseconds wait_ms);

  int process_event_loop (std::chrono::milliseconds wait_ms)
  {
    while (is_runing ())
    {
      if (-1 == process_event (wait_ms)) return -1;
    }
    return 0;
  }

  void stop_event_loop () { _is_stop = true; }

 public:
  //无法重复添加
  //保证对应sockslot 已经设置正确
  //当错误发送时调用readable 来发现错误,而不提供错误函数
  int epoll_add_sock (int fd, int events, void *on_recvable, void *on_sendable, void *arg);

  // void epoll_del_sock(int fd);
  void epoll_del_sock (sockinfo_t *sinfo);

  bool is_add_sock(int fd);

 public:
  bool is_skip_current_event_process () const { return _is_skip_current_event_process; }

 public:
  //需要实现特殊策略时使用
  void async_raw_tcp_connect (const std::string &ip,
                              const uint16_t port,
                              on_tcp_conn_complete_fn_t,
                              on_tcp_conn_failed_fn_t,
                              void *user_arg = nullptr);


  int add_sendable_ev (int sock);
  int del_sendable_ev (int sock);
  bool is_add_sendable_ev (int sock) const { return sockpool.get_info (sock)->_is_set_epollout; }

 private:
  net_engine_t () = default;
  int init ();

 private:
  int make_udp_client_socket(const std::string &dip, const uint16_t dport);
  int make_udp_server_socket(const std::string &sip, const uint16_t sport);


 private:
  bool _is_stop = false;

  // epoll
 private:
  enum
  {
    MAX_EVENT_SIZE = 1024
  };

 private:
  int _efd = -1;
  int _current_fd;
  bool _is_process = false;
  bool _is_skip_current_event_process = false;
  epoll_event _events[MAX_EVENT_SIZE];
};


static inline net_engine_t *make_net_engine () { return net_engine_t::make (); }
static inline void unmake_net_engine (net_engine_t *&eng) { net_engine_t::unmake (eng); }

}

