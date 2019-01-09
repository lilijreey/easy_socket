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
#include <esock_engine.hpp>
#include <esock_tcp_connect.hpp>

using esock::net_engine_t;
using esock::tcp_server_t;


//
void on_tcp_listen_ok(net_engine_t *eng, tcp_server_t *svr, void *user_arg)
{
    //printf("echo on_conn_complete fd:%d no\n", sock);
  //eng->close_listen(sock);
}

void on_tcp_listen_failed(net_engine_t *eng, const std::string &ip, const uint16_t port, const int errno, void *user_arg)
{
  //eng->close_listen(sock);
}

void on_tcp_accept_conn_complete(net_engine_t *eng, tcp_server_t *svr, int sock, void *user_arg)
{
}

void on_tcp_accept_conn_failed(net_engine_t *eng, tcp_server_t *svr, int errno, void *user_arg)
{
  //eng->release_tcp_server(svr);
  //eng->close_conn(sock);
}

//class echo_client_t: tcp_conn_strage_base_t
//{
//  virtual void on_conn_complete() override {}
//  virtual void on_conn_failed() override {}
//}

//class EchoServerListener : tcp_listen_strage_base_t
//{
//  virtual on_listen_ok();
//  ...
//};


  void on_tcp_conn_complete(net_engine_t *eng, const char *ip, const uint16_t port, int sock, void *arg)
  {
    printf("echo on_conn_complete no %d\n", sock);
    //eng->close_connect(sock);
  }

  void on_tcp_conn_failed(net_engine_t *eng,  const char *ip, const uint16_t port, const int err, void *arg)
  {
    printf("echo on_conn_failed no %d:%s\n", err, strerror(err));
    //eng->close_connect(sock);
  }

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

class EchoServer
{

  on_tcp_accept_conn_complete,
      on_tcp_accept_conn_failed);
}

  
int main()
{
  esock::net_engine_t *eng= esock::make_net_engine();

  assert(eng);

  EchoClient *client = new EchoClient();
  eng->async_tcp_connect("127.0.0.1", 5566,  client);


  EchoServer *server = new EchoServer();
  if (-1 == eng->async_tcp_server("127.0.0.1", 3344, server))
  {

  }


  //process events
  int ret = eng->process_event_loop(std::chrono::milliseconds(1000));

  esock::unmake_net_engine(eng);

}
