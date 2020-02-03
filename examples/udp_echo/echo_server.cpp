
#include <unordered_set>
#include <memory>
#include <esock.hpp>

using esock::net_engine_t;
using esock::tcp_listener_t;


class echo_server_t
  : public esock:: async_udp_handler_t<echo_server_t>
{

 public:
  ~echo_server_t()
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
    printf("udp recv:%s", data);
    if (datalen)
      ::sendto(_sock, data, datalen, 0, (sockaddr*)peeraddr, addrlen);
  }


  void on_error_occur(net_engine_t *eng, int sock, int error)
  {
    printf("udp has error :%d",sock);
  }

  std::pair<char*, ssize_t> get_recv_buff() {
    return {_buf, 64};
  }

  int _sock;
  char _buf[64];

};

int main()
{
  std::unique_ptr<net_engine_t> eng(esock::make_net_engine());

  echo_server_t svr;

  svr._sock = eng->add_async_udp_server("127.0.0.1", 5566, &svr);
  assert(svr._sock != -1);
  //svr._sock = svr.make_udp_server_socket("127.0.0.1", 5566);
  //eng->add_async_udp_server(&svr);

  eng->process_event_loop(std::chrono::milliseconds(1000));

  //auto destry eng
}
