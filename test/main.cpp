/**
 * @file     main.cpp
 *           
 *
 * @author   lili  <lilijreey@gmail.com>
 * @date     01/05/2019 01:28:23 PM
 *
 */
#include <cstdio>
#include <cassert>
#include <esock.hpp>

using esock::net_engine_t;
using esock::tcp_listener_t;



class EchoClient :
    public esock::async_tcp_client_hanndler_t<EchoClient>
{
  int sock;
  int port;

  char buf[1024];
 public:
  std::pair<char*, ssize_t> get_buffer_info() {
    return {buf, 64};
  }

  std::pair<char*, ssize_t> get_send_data() {
    return {buf, 1024};
  }

  void on_conn_complete(net_engine_t *eng, const char *ip, const uint16_t port, int sock)
  {
    //eng->watch_tcp_connect(sock, this);
    //eng->unwatch_tcp_connect();

    printf("echo on_conn_complete fd%d\n", sock);
    //TODO 要避免copy, 发送者就需要控制buf

  }

  void on_conn_failed(net_engine_t *eng, const char *ip, const uint16_t port, const int err)
  {
    printf("echo on_conn_failed %d:%s\n", err, strerror(err));
  }

  void on_recv_complete(net_engine_t *eng, int sock, const char *data, int datalen)
  {
    //printf("on recv comp fd:%d %s len:%d\n", sock, data, datalen);
    for (int i=0; i<(4<<10); ++i)
      post_send(eng, sock);

  }

  void on_send_complete(net_engine_t *eng, int sock, const char *data, int datalen)
  {
    printf("on send comp fd:%d %s len:%d\n", sock, data, datalen);
  }

  void on_peer_close(net_engine_t *eng, int sock)
  {
    printf("on peer close fd:%d\n", sock);
    eng->close_socket(sock);
  }

  void on_error_occur(net_engine_t *eng, int sock, int error)
  {
    printf("on error fd:%d, error%s\n", sock, strerror(error));
    eng->close_socket(sock);
  }

};

class EchoServerConn : public esock::tcp_server_conn_handler_t<EchoServerConn>
{
 public:
  int sock;
  int port;

  char buf[1024];
 public:
  std::pair<char*, ssize_t> get_buffer_info() {
    return {buf, 64};
  }

  std::pair<char*, ssize_t> get_send_data() {
    return {buf, 1024};
  }


  void on_recv_complete(net_engine_t *eng, int sock, const char *data, int datalen)
  {
    //printf("on recv comp fd:%d %s len:%d\n", sock, data, datalen);
    for (int i=0; i<(4<<10); ++i)
      post_send(eng, sock);

  }

  void on_send_complete(net_engine_t *eng, int sock, const char *data, int datalen)
  {
    printf("on send comp fd:%d %s len:%d\n", sock, data, datalen);
  }

  void on_peer_close(net_engine_t *eng, int sock)
  {
    printf("on peer close fd:%d\n", sock);
    eng->close_socket(sock);
  }

  void on_error_occur(net_engine_t *eng, int sock, int error)
  {
    printf("on error fd:%d, error%s\n", sock, strerror(error));
    eng->close_socket(sock);
  }

};

class EchoServer : public esock::tcp_server_handler_t<EchoServer, EchoServerConn>
{
  //tcp_listener_t *listener;
  //tcp_server_connect_t::on_recv_complete()

 public:
  EchoServerConn* on_accept_complete(net_engine_t *eng, tcp_listener_t *ins, int conn, sockaddr_storage addr)
  {
    esock_debug_log("new conn fd:%d", conn);

    return nullptr;
  }

  void on_accept_failed(net_engine_t *eng, tcp_listener_t *listener, int error)
  {
    esock_debug_log("on_accept failed fd:%d", listener->get_sock());
  }

};


  
int main()
{
  esock::net_engine_t *eng= esock::make_net_engine();

  assert(eng);

  //EchoClient *client = new EchoClient();
  //eng->async_tcp_connect("127.0.0.1", 5566,  client);


  tcp_listener_t *l= esock::make_tcp_listener("127.0.0.1", 5566);
  assert(l);


  EchoServer *server = new EchoServer();
  eng->add_tcp_server(l, server);

  l->start_listen();
  printf("listen ok\n");


  //process events
  int ret = eng->process_event_loop(std::chrono::milliseconds(1000));

  esock::unmake_net_engine(eng);

  return 0;

}
