/**
 * @file     echo_client.cpp
 *
 * @author   lili  <lilijreey@gmail.com>
 * @date     01/13/2019 11:49:29 PM
 *
 */


#include <esock.hpp>
using esock::net_engine_t;




class echo_client_t : public esock::async_udp_handler_t<echo_client_t>
{
public:
  ~echo_client_t()
  {
    if (_sock != -1)
    {
      esock::close_socket(_sock);
      _sock = -1;
    }
  }

  void on_recv_complete(net_engine_t *eng, int sock, const char *data, size_t datalen, 
                        sockaddr_storage *peeraddr, socklen_t addrlen)
  {
    printf("udp recv:%s %d", data, ++_cnt);
    send();
  }

  void send()
  {
      ::send(_sock, "hello\n", 6, 0);
  }

  void on_error_occur(net_engine_t *eng, int sock, int error)
  {
    printf("udp has error :%d",sock);
  }

  std::pair<char*, ssize_t> get_recv_buff() {
    return {_buf, 64};
  }

  int _sock;
  int _cnt{};
  char _buf[64];

};



int main()
{
  echo_client_t *client = new echo_client_t;

  net_engine_t *eng = esock::make_net_engine();
  int sock = eng->add_async_udp_client("127.0.0.1", 5566, client);
  assert(sock != -1);
  client->_sock = sock;
  client->send();
  

  eng->process_event_loop(std::chrono::milliseconds(1000));

  delete client;// and test timeout failed;

  esock::unmake_net_engine(eng);
}

