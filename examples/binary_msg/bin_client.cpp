/**
 * @file     echo_client.cpp
 * 
 */




#include <vector>
#include <esock.hpp>
#include "package_buffer.hpp"

#include "protocol.hpp"


using esock::net_engine_t;


//class BinClient : public esock::tcp_client_t<BinClient, proto_head_t, 4<<10>
class BinClient : public esock::async_tcp_client_hanndler_t<BinClient>
                  ,public esock::tcp_client_t<BinClient, proto_head_t, 4 <<10>
{
 private:
  std::string svrIp;
  uint16_t svrPort;
  enum {MAX_BUF=64};
  char sendBuf[MAX_BUF];
  char recvBuf[MAX_BUF];
  int sendPos=0;
  int sendDataLen=0;
  int recvPos=0;
  int _sock=-1;
  int _addCount;
  std::vector<uint64_t> _results;

public:
  ~BinClient()
  {
      if (_sock != -1)
      {
          printf("destroy close sock\n");
          esock::close_socket(_sock);
      }
  }

 public: //callback
   void on_conn_connecting(net_engine_t *eng, const char *ip, const uint16_t port, int sock)
   {
       assert(sock != -1);
       printf("on connectiong fd:%d\n", sock);
       _sock = sock;
   }

   void on_conn_complete(net_engine_t *eng, const char *ip, const uint16_t port, int sock)
   {
   }
 
   void on_conn_failed(net_engine_t *eng, const char *ip, const uint16_t port, const int err)
   {
     printf("on conn %s:%d failed reconnct %s\n", ip, port, strerror(err));
     sleep(1);
     //reconnect
     //eng->async_tcp_client("127.0.0.1", 5566, this);
   }

   void on_recv_pkg_invalid(net_engine_t *eng, int sock, proto_head_t *head)
   {
     printf("get an invalid pkg head, close sock:%d\n", sock);
     eng->close_socket(sock);
   }

   void on_recv_pkg_complete(net_engine_t *eng, int sock, const proto_head_t *msg, const size_t bodylen)
   {
       //TODO handle msg

   }
 
   void on_peer_close(net_engine_t *eng, int sock)
   {
     printf("sock:%d peer closed, close local, reconnect\n", sock);
     eng->close_socket(sock);
     sleep(1);
     eng->async_tcp_client("127.0.0.1", 5566, this);
     //reconnect
   }
 
   void on_error_occur(net_engine_t *eng, int sock, int error)
   {
     printf("sock:%d error, %s ", sock, strerror(error));
     eng->close_socket(sock);
   }

       //build number and send to server
#if 0
       _addCount = random() % 512;
       _resultes.reverse(_addCount);

       _sendbuf.empty();

       //size_t pos = _sendbuf.append(_addCount);
       //_sendbuf.visit_pos(pos);
       
       //build proto_add_request_t msg
       _sendbuf.get_writable_size();

       if (not _sendbuf.is_can_write(sizeof(proto_add_request_t) + sizeof(proto_add_request_t::pair) * _addCount))
       {
           //fix len
       }

       
       req->pair_couont = _addCount;
       for (int i=0; i < _addCount; ++i) {
           req->[i].num1 = random();
           req->[i].num2 = random();

           _results.push_back((int64_t)(req->[i].num1) + req->[i].num2);
       }                                

       _sendbuf.package();
       //攒够1K在发送
       if (_sendbuf.sendable_len() > _1K)
       {
           post_send(eng, sock);
           //post_send(sock);
       }
#endif

 
};


int main()
{
  BinClient *client = new BinClient;

  net_engine_t *eng = esock::make_net_engine();
  eng->async_tcp_client("127.0.0.1", 5566, client);
  //printf("delete client\n");
  //delete client;

  eng->process_event_loop(std::chrono::milliseconds(1000));

  delete client;// and test timeout failed;

  esock::unmake_net_engine(eng);
}

