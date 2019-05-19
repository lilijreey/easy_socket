/**
 * @file     echo_client.cpp
 * 
 */




#include <vector>
#include <esock.hpp>
#include <buffer/esock_netbuff.hpp>

#include "protocol.hpp"


using esock::net_engine_t;


class BinClient 
  : public esock::tcp_active_conn_t<BinClient, proto_head_t, 4 <<10>
{
 private:
  int _sock=-1;
  int _count;
  std::vector<int64_t> _results;

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
   void on_conn_connecting(net_engine_t *eng, int sock)
   {
       assert(sock != -1);
       printf("on connectiong fd:%d\n", sock);
       _sock = sock;
   }

   void on_conn_complete(net_engine_t *eng, int sock)
   {
       send_add_req(eng);
   }
 
   void on_conn_failed(net_engine_t *eng, const int err)
   {
     printf("on conn %s:%d failed reconnct %s\n", "xx", 5566, strerror(err));
     sleep(1);
     //reconnect
     eng->async_tcp_client("127.0.0.1", 5566, this);
   }

   void on_recv_pkg_invalid(net_engine_t *eng, int sock, const proto_head_t *head)
   {
     printf("get an invalid pkg head, stop event loop close sock:%d\n", sock);
     eng->close_socket(sock);
     _sock = -1;
     eng->stop_event_loop();
   }

   //业务逻辑
   void send_add_req(net_engine_t *eng)
   {
       //build number and send to server
       _count = random() % 500;
       _results.reserve(_count);

       assert(_sendbuf.is_empty_pkg_body());

       size_t body_len = (_count * sizeof(uint32_t) * 2 + sizeof(uint32_t));// pair_count

       if (body_len > _sendbuf.get_writable_len())
       {
           auto count = _sendbuf.get_writable_len() / (sizeof(uint32_t) * 2);
           assert(count >=2);
           --count;
           printf("sendbuff no space, change count:%d:%ld\n", _count, count);
           _count = count;
           body_len = (_count * sizeof(uint32_t) * 2 + sizeof(uint32_t));// pair_count
       }

       _results.clear();
       //push packge
       proto_add_request_t *req = _sendbuf.get_pkg_head<proto_add_request_t>();
       req->head.magic = PROTO_MAGIC;
       req->head.cmd = PROTO_ADD_CMD_REQ;
       req->head.pkglen = sizeof(proto_head_t) + body_len;
       req->pair_count = _count;
       for (int i=0; i < _count; ++i) {
           req->pair[i].num1 = random();
           req->pair[i].num2 = random();

           _results.push_back((int64_t)(req->pair[i].num1) + req->pair[i].num2);
       }                                
       _sendbuf.increase_write_len(body_len);

       _sendbuf.make_next_pkg();
       post_send(eng, _sock);
       printf("send count:%d\n", _count);
   }

   void handle_add_rsp(net_engine_t *eng, const proto_add_response_t *rsp, const size_t bodylen)
   {
       //接受结果，然后和本地结果比对，并发送新的请求
       if (rsp->count != _results.size())
       {
           printf("recv rsp count:%u, but not same local:%lu\n", rsp->count, _results.size());
           eng->close_socket(_sock);
           _sock = -1;
           eng->stop_event_loop();
           return;
       }

       for (size_t i=0; i < rsp->count; ++i) {
           assert(rsp->resluts[i] == _results[i]);
       }
       printf("recv result cnt :%u check ok\n", rsp->count);


       send_add_req(eng);
   }


   void on_recv_pkg_complete(net_engine_t *eng, int sock, const proto_head_t *head, const size_t bodylen)
   {
       switch (head->cmd) {
       case PROTO_ADD_CMD_RSP:
           handle_add_rsp(eng, (proto_add_response_t*)head, bodylen);
           //eng->delete_this(this);
           //eng->stop_event_loop();
           return;
       default:
           printf("bad cmd:%d\n", head->cmd);
           return;
       }
   }
 
   void on_peer_close(net_engine_t *eng, int sock)
   {
     printf("sock:%d peer closed, close local, reconnect\n", sock);
     eng->close_socket(sock);
     sleep(1);
     //eng->async_tcp_client("127.0.0.1", 5566, this);
     //reconnect
   }
 
   void on_error_occur(net_engine_t *eng, int sock, int error)
   {
     printf("sock:%d error, %s ", sock, strerror(error));
     eng->close_socket(sock);
     _sock = -1;
     eng->stop_event_loop();
   }


 
};


int main()
{
  BinClient *client = new BinClient;

  net_engine_t *eng = esock::make_net_engine();
  eng->async_tcp_client("127.0.0.1", 5566, client);
  //printf("delete client\n");
  //delete client;

  eng->process_event_loop(std::chrono::milliseconds(1000));

  //delete client;// and test timeout failed;

  esock::unmake_net_engine(eng);
}

