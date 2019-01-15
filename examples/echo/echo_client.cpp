/**
 * @file     echo_client.cpp
 *
 * @author   lili  <lilijreey@gmail.com>
 * @date     01/13/2019 11:49:29 PM
 *
 */


#include <esock.hpp>
using esock::net_engine_t;

class EchoClient 
  : public esock::async_tcp_client_hanndler_t<EchoClient>
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

public:
  ~EchoClient()
  {
      if (_sock != -1)
          esock::close_socket(_sock);
  }

 public: //callback
   void on_conn_connecting(net_engine_t *eng, const char *ip, const uint16_t port, int sock)
   {
       assert(sock != -1);
       _sock = sock;
   }

   void on_conn_complete(net_engine_t *eng, const char *ip, const uint16_t port, int sock)
   {
     printf("on conn %s:%d ok sock:%d\n", ip, port, sock);
     sendPos = 0;
     //send hello word\n
     const char *msg = "hello world\n";
     sendDataLen = strlen(msg);
     memcpy(sendBuf, msg, sendDataLen);
     post_send(eng, sock);
   }
 
   void on_conn_failed(net_engine_t *eng, const char *ip, const uint16_t port, const int err)
   {
     printf("on conn %s:%d failed reconnct %s\n", ip, port, strerror(err));
     sleep(1);
     //reconnect
     eng->async_tcp_client("127.0.0.1", 5566, this);
   }
 
   void on_recv_complete(net_engine_t *eng, int sock, const char *data, const ssize_t datalen)
   {
     //解包
     //最大message len 63
     //以\n结尾为一个message
     recvPos += datalen;
     if (recvPos == MAX_BUF || data[datalen-1] == '\n')
     {
       ((char*)data)[recvPos]='\0';
       printf("recv msg:%s len:%d\n", recvBuf, recvPos);
       recvPos = 0;
     }
   }
 
   void on_send_complete(net_engine_t *eng, int sock, const char *data, const ssize_t sendlen)
   {
       sendPos += sendlen;
       if (sendPos < sendDataLen)
       {
           printf("continue send :%d", sendPos);
       }
       assert(sendPos <= sendDataLen);
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

   //返回需要发送的数据
   std::pair<char*, ssize_t> get_send_data()
   {
     return {sendBuf + sendPos, sendDataLen - sendPos};
   }
 
   //需要处理buff满的情况
   std::pair<char*, ssize_t> get_recv_buff() 
   {
     return {recvBuf+ recvPos, MAX_BUF-recvPos};
   }
 
};


int main()
{
  EchoClient client;

  net_engine_t *eng = esock::make_net_engine();
  eng->async_tcp_client("127.0.0.1", 5566, &client);

  eng->process_event_loop(std::chrono::milliseconds(1000));

  esock::unmake_net_engine(eng);
}

