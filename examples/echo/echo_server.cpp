
#include <unordered_set>
#include <memory>
#include <esock.hpp>

using esock::net_engine_t;
using esock::tcp_listener_t;

class EchoServer;

//接受\n结尾字符，然后发送回去
class EchoServerConn
  : public esock::tcp_server_conn_handler_t<EchoServerConn>
{
 public:
    EchoServerConn(int sock, EchoServer *svr) 
        :_sock(sock),_svr(svr) {}
    ~EchoServerConn()
    {
        if (_sock) {
            esock::close_socket(_sock);
            _sock = -1;
        }
    }
 public:
    //callback
   void on_recv_complete(net_engine_t *eng, int sock, const char *data, const ssize_t datalen);
 
   //发送完成数据是通知,一般用来更新buff
   void on_send_complete(net_engine_t *eng, int sock, const char *data, const ssize_t sendlen)
   {
       sendPos += sendlen;
       if (sendPos == recvPos)
           sendPos = recvPos = 0;
   }
 
   void on_peer_close(net_engine_t *eng, int sock);
   void on_error_occur(net_engine_t *eng, int sock, int error);
 
   //需要处理buff满的情况
   std::pair<char*, ssize_t> get_recv_buff() 
   {
       printf("get recv buf\n");
       return {buf+recvPos, MAX_BUF-recvPos};
   }
 
   //返回需要发送的数据
   std::pair<char*, ssize_t> get_send_data() 
   {
       return {buf+sendPos, recvPos-sendPos};
   }
 private:
    enum {MAX_BUF=64};
    int _sock=-1;
    EchoServer *_svr;
    char buf[MAX_BUF];
    int recvPos=0;
    int sendPos=0;
};

class EchoServer 
  : public esock::tcp_server_handler_t<EchoServer, EchoServerConn>
{
 private:
    esock::tcp_listener_t *_listener=nullptr;
    std::unordered_set<EchoServerConn*> _conns;

 public:
  bool init(const std::string &ip, uint16_t port)
  {
      _listener = esock::make_tcp_listener(ip, port);
      return _listener != nullptr;
  }

  ~EchoServer() {
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

  void closeConn(EchoServerConn *conn)
  {
      _conns.erase(conn);
      delete conn;
  }

 public:
  //callback
  EchoServerConn*
  on_accept_complete(net_engine_t *eng, tcp_listener_t *ins, int sock, sockaddr_storage addr)
  {
      printf("accept new conn fd %d\n", sock);
      EchoServerConn *conn = new EchoServerConn(sock, this);
      _conns.insert(conn);
      return conn;
  }

  void on_accept_failed(net_engine_t *eng, tcp_listener_t *ins, int error)
  {
      printf("accept conn failed %s\n", strerror(error));
  }
      
};


void EchoServerConn::on_recv_complete(net_engine_t *eng, int sock, const char *data, const ssize_t datalen)
{
    assert(datalen>0);
    recvPos += datalen;
    if (data[datalen-1] == '\n')
    {
        if (strncmp(data, "close server\n", strlen("close server\n")) == 0)
            eng->stop_event_loop();

        assert(sendPos == 0);
        //TODO disabel recv
        post_send(eng, sock);
    } 
    else if (recvPos == MAX_BUF)
    {//msg too long
        printf("sock %d msg too long, close socket", sock);
        _svr->closeConn(this);
        return;
    }
}

 
void EchoServerConn::on_peer_close(net_engine_t *eng, int sock)
{
    printf("conn fd:%d peer closed\n", sock);
    _svr->closeConn(this);
}

void EchoServerConn::on_error_occur(net_engine_t *eng, int sock, int error)
{
    printf("conn fd:%d error:%s\n", sock, strerror(error));
    _svr->closeConn(this);
}

int main()
{
  std::unique_ptr<net_engine_t> eng(esock::make_net_engine());

  EchoServer svr;
  if (not svr.init("127.0.0.1", 5566))
  {
      exit(-1);
  }

  eng->add_tcp_server(svr.get_listener(), &svr);

  svr.listen();

  eng->process_event_loop(std::chrono::milliseconds(1000));

  //auto destry eng
}
