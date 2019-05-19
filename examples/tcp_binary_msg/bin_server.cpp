
#include <unordered_set>
#include <memory>
#include <esock.hpp>
#include <buffer/esock_netbuff.hpp>
#include "protocol.hpp"

int times = 10;
using esock::net_engine_t;
using esock::tcp_listener_t;

class BinServer;

//接受\n结尾字符，然后发送回去
class BinServerConn
  : public esock::tcp_passive_conn_t<BinServerConn, proto_head_t, 4<<10>
{
 
 private:
    int _sock=-1;
    BinServer *_svr;
 public:
    BinServerConn(int sock, BinServer *svr) 
        :_sock(sock),_svr(svr) {}
    ~BinServerConn()
    {
        if (_sock) {
            esock::close_socket(_sock);
            _sock = -1;
        }
    }
 public:
    //callback

    void handle_add_req(net_engine_t *eng, const proto_add_request_t* req, const size_t reqBodylen)
    {
        //计算num1 + num2 写入sendbuff 发送
        assert(req->pair_count < 500);
        assert(_sendbuf.is_empty_pkg_body());

        //push response
        const size_t bodylen = req->pair_count * sizeof(int64_t) + sizeof(uint32_t);//count
        if (not _sendbuf.ensure_writable_len(bodylen))
        {
            printf("buff is full, drop data\n");
            return;
        }

        if (_sendbuf.get_writable_len() < bodylen)
        {
            _sendbuf.discard_sended_data();
            if (_sendbuf.get_writable_len() < bodylen)
            {
            }
        }
        auto *rsp = _sendbuf.get_pkg_head<proto_add_response_t>();
        rsp->head.magic = PROTO_MAGIC;
        rsp->head.cmd   = PROTO_ADD_CMD_RSP;
        rsp->head.pkglen= sizeof(proto_head_t) + bodylen;;
        rsp->count      = req->pair_count;

        for (size_t i=0; i< req->pair_count; ++i) {
            rsp->resluts[i] = (int64_t)(req->pair[i].num1) + req->pair[i].num2;
        }
       _sendbuf.increase_write_len(bodylen);

       _sendbuf.make_next_pkg();
       post_send(eng, _sock);
       printf("send count:%d\n",rsp->count);
       //eng->skip_current_event_process();
       //delete(this);
       //if (--times == 0)
       //    eng->stop_event_loop();

    }

    void on_recv_pkg_complete(net_engine_t *eng, int sock, const proto_head_t *head, const size_t bodylen)
    {
       switch (head->cmd) {
       case PROTO_ADD_CMD_REQ:
           handle_add_req(eng, (proto_add_request_t*)head, bodylen);
           return;
       default:
           printf("bad cmd:%d\n", head->cmd);
           return;
       }
    }
 
   void on_recv_pkg_invalid(net_engine_t *eng, int sock, const proto_head_t *head);
   void on_peer_close(net_engine_t *eng, int sock);
   void on_error_occur(net_engine_t *eng, int sock, int error);
};

class BinServer 
  : public esock::tcp_server_accept_handler_t<BinServer, BinServerConn>
{
 private:
    esock::tcp_listener_t *_listener=nullptr;
    std::unordered_set<BinServerConn*> _conns;

 public:
  bool init(const std::string &ip, uint16_t port)
  {
      _listener = esock::make_tcp_listener(ip, port);
      return _listener != nullptr;
  }

  ~BinServer() {
      esock::unmake_tcp_listener(_listener);
      for (auto conn : _conns)
      {
          delete conn;
          conn = nullptr;
      }
      _conns.clear();
  }

  tcp_listener_t* get_listener() {return _listener;}

  void listen() {_listener->start_listen();}

  void closeConn(BinServerConn *conn)
  {
      _conns.erase(conn);
      delete conn;
  }

 public:
  //callback
  BinServerConn*
  on_accept_complete(net_engine_t *eng, tcp_listener_t *ins, int sock, sockaddr_storage addr)
  {
      printf("accept new conn fd %d\n", sock);
      BinServerConn *conn = new BinServerConn(sock, this);
      _conns.insert(conn);
      return conn;
  }

  void on_accept_failed(net_engine_t *eng, tcp_listener_t *ins, int error)
  {
      printf("accept conn failed %s\n", strerror(error));
  }
      
};


 
void BinServerConn::on_peer_close(net_engine_t *eng, int sock)
{
    printf("conn fd:%d peer closed\n", sock);
    _svr->closeConn(this);
}

void BinServerConn::on_error_occur(net_engine_t *eng, int sock, int error)
{
    printf("conn fd:%d error:%s\n", sock, strerror(error));
    _svr->closeConn(this);
}

void BinServerConn::on_recv_pkg_invalid(net_engine_t *eng, int sock, const proto_head_t *head)
{
    printf("get an invalid pkg head, stop event loop close sock:%d\n", sock);
    _svr->closeConn(this);
}

int main()
{
  std::unique_ptr<net_engine_t> eng(esock::make_net_engine());

  BinServer svr;
  if (not svr.init("127.0.0.1", 5566))
  {
      exit(-1);
  }

  eng->add_tcp_server(svr.get_listener(), &svr);

  svr.listen();

  eng->process_event_loop(std::chrono::milliseconds(1000));

  //auto destry eng
}
