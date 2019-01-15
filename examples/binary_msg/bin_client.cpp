/**
 * @file     echo_client.cpp
 * 
 */




#include <vector>
#include <esock.hpp>

#include "protocol.hpp"
using esock::net_engine_t;


//class BinClient : public tcp_client_package_ability<BinClient>
//在IO层上实现一种简单的buff管理策略+解包策略,提供给业务侧使用,
//当然你也可以定义自动的buffer类,和解包策略
//一定要结合async_tcp_client_hanndler_t
//或者tcp_server_conn_handler 使用

template <typename pkg_head_t, size_t BUFSIZE, size_t PKG_HEAD_LEN=sizeof(pkg_head_t)>
struct recv_pkg_sbuff_t// like netty channel
{
    enum {COOKIE=0xcccccccc};

    size_t get_capacity() const {return BUF_SIZE;}

    uint32_t get_readable_len() const 
    { 
        assert(_wpos >= _rpos);
        return _wpos - _rpos; 
    }
    uint32_t get_writeable_len() const
    {
        assert(BUFSIZE >= _wpos);
        return BUFSIZE - _wpos;
    }

    char* get_writeable_buff()
    {
        return _buf + _wpos;
    }

    //不检验参数，请保证参数有效
    //消费数据
    const pkg_head_t* read_package(size_t len) 
    { 
        void *data = _buf + _rpos;
        assert(get_readable_len() >= len);
        _rpos += len;
        assert(_rpos <= get_capacity());
        if (_rpos == _wpos) _rpos = _wpos;
        return data;
    }


    const pkg_head_t* peek_package() const 
    {
        return _buf + _rpos;
    }

    //返回清理的空间大小
    uint32_t discard_readed_data()
    {
        const uint32_t readedLen = _rpos;
        const uint32_t unreadLen = get_readable_len();
        if (unreadLen != 0)
        {
          memmove(_buf, _buf+_rpos, unreadLen);
        }
        _rpos = 0;
        _wpos = unreadLen;
        return readedLen;
    }


    inline void reset()
    {
        _rpos=0;
        _wpos = 0;
    }

    bool is_wrire_out_bounds() const
    {
        return _cookie1 == COOKIE or _cookie2 == COOKIE;
    }


    ////如果头不足返回nullptr
    const pkg_head_t* peek_package_head() const
    {
        if (this->get_readable_len() < PKG_HEAD_LEN)
        {
            return nullptr;
        }
        return peek_package();
    }


    bool is_has_complete_pkg(const size_t pkgLen) const
    {
        return get_readable_len() >= pkgLen;
    }

    //
    //   +----------+---------------+--------------+
    //   |readedData| readableData  | writeableLen |
    //   +----------+---------------+--------------+
    //             /             /                /
    //          _rpos         _wpos     BUF_SIZE 最大长度
    // _rpos 永远指向一个包的头部
    //   
    uint32_t _cookie1     = COOKIE; //write protectd
    uint32_t _wpos        =0;	//可写入位置
    uint32_t _rpos        =0;  //未读数据位置 _rpos <= _wpos
    char     _buf[BUFSIZE];
    uint32_t _cookie2     = COOKIE; //write protectd
};


template <typename PkgHead, size_t BUFSIZE, size_t PKG_HEAD_LEN=sizeof(PkgHead)>
struct net_send_buf_t
{


  //
  //                     m_pkgHeaderPos
  //                                 /  
  //                  | sendableLen  | pkgLen| 
  //   +---+----------+--------------+-------+--------------+
  //   |   |sendedData| readableData         | writeableLen |
  //   +---+----------+--------------+-------+--------------+
  //              /                         /              /
  //      _rpos                      _wpos     BUF_SIZE 最大长度
  // 头部预留PKG_HEAD_LEN长度的空间,利于打包
  // _rpos 已发送数据的下一个
  // _hpos  指向正在写入的数据的包头，_hpos指向的数据不会被发送 
  // 如果打包后空间无容纳新的包头，则_hpos= _wpos,这时不可写，也没有包
  // 可用IsHasP
  // _wpos 指向下一个将写入的地址
  //   

  enum {COOKIE=0xcccccccc};
  uint32_t _cookie1 = COOKIE;
  uint32_t _rpos         =0;
  uint32_t _hpos          =0;	
  uint32_t _wpos        =PKG_HEAD_LEN;	//预留一个包头
  char m_buf[BUFSIZE];
  uint32_t m_cookie2 = COOKIE;

  inline void reset()
  {
      _rpos=0;
      _hpos=0;
      _wpos = PKG_HEAD_LEN;
  }

  inline size_t GetCapacity()
  {
      return BUFSIZE;
  }

   uint32_t GetPkgHeadLen() const
  {
      return PKG_HEAD_LEN;
  }

  bool IsHasPkg() const
  {
      return _hpos != _wpos;
  }


  uint32_t GetPkgLen() const
  {
    assert(_wpos >= _hpos);
    return _wpos - _hpos;
  }

  uint32_t GetPkgBodyLen() const
  {
      if (_hpos == _wpos)
          return 0; //无空间容纳新包

      assert(_wpos >= _hpos + PKG_HEAD_LEN);
      return _wpos - _hpos - PKG_HEAD_LEN;
  }

  bool IsEmptyPkgBody() const
  {
      return _wpos - _hpos == PKG_HEAD_LEN;
  }

  uint32_t GetSendableLen() const
  {
    ASSERT(_hpos >= _rpos);
    return (_hpos - _rpos);
  }

  const void *GetSendableData()
  {
    return (void*)_rpos;
  }

  uint32_t GetWritableLen() const
  {
      if (_wpos == _hpos)
          return 0;

      assert(_wpos <= BUFSIZE);
      return BUFSIZE - _wpos;
  }


  PkgHead *GetPkgHead()
  {
      assert(IsHasPkg());
    return (PkgHead *)(m_buf + _hpos);
  }

  void* GetPkgBody()
  {
      assert(IsHasPkg());
    return (void*)(m_buf + _hpos + PKG_HEAD_LEN);
  }

  //void* GetWriteBuf

  inline size_t DiscardSendedData()
  {
    ASSERT(_wpos > _rpos);
    assert(_wpos <= BUFSIZE);

    const uint32_t sendedLen = _rpos;
    if (sendedLen == 0)
        return 0;

    //TODO 超过最低水平线在移动
    memmove(m_buf, m_buf+_rpos, _wpos - _rpos);

    _rpos = 0;
    if (_wpos == _hpos && _wpos + PKG_HEAD_LEN - sendedLen <= BUFSIZE) {
        _hpos -= sendedLen;
        _wpos -= sendedLen;
        _wpos += PKG_HEAD_LEN;
    }
    else
    {
        _hpos -= sendedLen;
        _wpos -= sendedLen;
    }

    assert(_wpos <= BUFSIZE);
    return sendedLen;
  }


  //丢弃到从上次MakeNextPkg后写入的数据
  //返回丢弃的PkgBody长度
  uint32_t DiscardPkgData()
  {
      assert(IsHasPkg());
      uint32_t discardLen = _wpos - _hpos - PKG_HEAD_LEN;
      _wpos = _hpos + PKG_HEAD_LEN;
      return discardLen;
  }


  //调用后之前的数据将允许被发送,一般在打包后调用
  //预留一个PKG_HEAD_LEN长度空间，可以使用GetPkgHead返回包头指针
  //如果后续空间不足HEAD_LEN则设置_hpos = _wpos 
  bool MakeNextPkg()
  {
    if (GetPkgLen() <= PKG_HEAD_LEN)
    {
      LOG_ERR("make a empty pkg");
      return false; //empty head
    }

    assert(_wpos <= BUFSIZE);
    _hpos = _wpos;
    if (_wpos + PKG_HEAD_LEN <= BUFSIZE) {
        _wpos += PKG_HEAD_LEN;
        return true;
    }
    return false;
  }



  //压入一个值类型，不支持数组类型，指针类型以免出错
  //返回写入的字节数
  template <typename T>
  size_t AppendValue(T&& a)
  {
    static_assert(std::is_pod<std::remove_reference<T>>::value, "need POD type");
    static_assert(not std::is_array<std::remove_reference<T>>::value, "can not pass array type");
    static_assert(not std::is_pointer<std::remove_reference<T>>::value, "can not pass pointer type");
    //assert(GetWritableLen() >= sizeof(T));
    //printf("append &&T size :%lu\n", sizeof(T));

    *( typename std::remove_const<typename std::remove_reference<T>::type>::type *)(m_buf + _wpos) = a;
    _wpos += sizeof(T);
    assert(_wpos <= BUFSIZE);
    return sizeof(T);
  }

  //不支持指针,数组，请使用AppendBuf
  template <typename T, size_t LEN> size_t AppendValue(T (&arr)[LEN]);
  template <typename T> size_t AppendValue(T* a);

  //支持可变参数 Append(3,54, 32L, obj) 参数需要是POD类型
  template<typename T, typename ...Args>
  size_t AppendValue(T&& a, Args... args)
  {
    return AppendValue(std::forward<T>(a)) + AppendValue(args...);
  }

  //写buffer数据
  //主要用来区分指针类型,到底是压指针的值还是指针指向的空间
  size_t AppendBuffer(const void *data, const size_t dataLen)
  {
    ASSERT_RET(GetWritableLen() >= dataLen, 0);

    memcpy(m_buf+_wpos, data, dataLen);
    _wpos += dataLen;
    assert(_wpos <= BUFSIZE);
    return dataLen;
  }


  //发送SendBuf中的数据
  //下发成功返回true,false表示出现错误，需要关闭对象
  //ret -1 表示发送失败，需要关闭fd
  //n 表示发送字节数
  //sendLen [out] 发送的长度
  ssize_t SendPkg(int fd)
  {
    //发送给定长度数据，直到发送完毕(非阻塞),或者返回EAGIN,或者出错
    ssize_t ret = NetSend(fd, m_buf + _rpos, GetSendableLen(), MSG_NOSIGNAL);
    //LOG_DBG("fd %d send %ld", fd, ret);
    if (ret == -1)
    {
      return -1;
    }
    _rpos += ret;
    ASSERT(_rpos <= _hpos);
    assert(_hpos <= BUFSIZE);

    //每次都整理位置，可优化
    DiscardSendedData();
    return ret;
  }

};

template <class subclass_t>
struct tcp_conn_sbuff
{

   //返回能给个写入的长度
   std::pair<char*, size_t> get_recv_buff() 
   {
       if (_recvbuf.get_writeable_len() < PKG_HEAD_LEN)
       {
           _recvbuf.discard_readed_data();
       }

       //TODO drop data

       return {
           _recvbuf.get_writeable_buff(),
           _recvbuf.get_writeable_len()
       };
   }

   void on_recv_complete(net_engine_t *eng, int sock, const char *data, const size_t datalen)
   {
       //接受的数据写入get_recv_buff 返回的空间,
       assert(datalen != 0);
       assert(_recvbuf._wpos + datalen <= _recvbuf.get_capacity());
       _recvbuf._wpos += datalen;

        while (const pkg_head_t *head = _recvbuf.peek_package_head())
        {
            //解析数据，调用子类HandleNetPkg
            //使用CRTP,实现编译期重载
            if (not head->is_valid_head())
            {
                //TODO 只检查一次
                static_cast<subclass_t*>(this)->on_recv_invalid_pkg(eng, sock, head);
                return;
            }

            if (not _recvbuf.is_complete_pkg(head->pkgLen))
                return;

            _recvbuf.read_package(head->pkgLen); //XXX 省略掉参数

            if (not static_cast<subclass_t*>(this)->on_recv_pkg_complete(eng, sock, head, head->pkgLen - PKG_HEAD_LEN))
                return;
        }
   }

   //返回需要发送的数据
   //自动实现
   std::pair<char*, ssize_t> get_send_data()
   {
       return {0,0};
       //_sendbuf.get_send_data();
   }
 
   //自动实现
   void on_send_complete(net_engine_t *eng, int sock, const char *data, const ssize_t sendlen)
   {
       //_sendbuf.custom_data(sendlen);
   }

   recv_pkg_sbuff _recvbuf;
 
   //需要处理buff满的情况
   //自动实现
};

class BinClient 
  : public esock::async_tcp_client_hanndler_t<BinClient>,
   public tcp_conn_sbuff<BinClient>
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
       //build number and send to server
#if 0
       _addCount = random() % 512;
       _resultes.reverse(_addCount);

       _sendbuf.empty();

       //size_t pos = _sendbuf.append(_addCount);
       //_sendbuf.visit_pos(pos);
       
       //build proto_add_request_t msg
       _sendbuf.get_writeable_size();

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
   }
 
   void on_conn_failed(net_engine_t *eng, const char *ip, const uint16_t port, const int err)
   {
     printf("on conn %s:%d failed reconnct %s\n", ip, port, strerror(err));
     sleep(1);
     //reconnect
     //eng->async_tcp_client("127.0.0.1", 5566, this);
   }

   on_recv_invalid_pkg(net_engine_t *eng, int sock, headPkg)
   {

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

