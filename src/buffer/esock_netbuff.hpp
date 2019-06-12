/**
 * @file     package_buffer.hpp
 *           
 *
 * @brief  在IO层上实现一种简单的buff管理策略+解包策略,提供给上层业务侧使用.
 *
 *
 */

#pragma once

namespace esock {
class net_engine_t;

//buf的长度应该是最大应用层包长的两倍
template <typename pkg_head_t, size_t BUFSIZE, size_t PKG_HEAD_LEN=sizeof(pkg_head_t)>
struct net_recvbuf_t// like netty channel
{
    //
    //   +----------+---------------+--------------+
    //   |readedData| readableData  | writableLen |
    //   +----------+---------------+--------------+
    //             /             /                /
    //          _rpos         _wpos     BUFSIZE 最大长度
    // _rpos 永远指向一个包的头部
    //   
    uint32_t _cookie1     = COOKIE; //write protectd
    uint32_t _wpos        =0;	//可写入位置
    uint32_t _rpos        =0;  //未读数据位置 _rpos <= _wpos
    char     _buf[BUFSIZE];
    uint32_t _cookie2     = COOKIE; //write protectd

 public:
    enum {COOKIE=0xcccccccc};

    size_t get_capacity() const {return BUFSIZE;}

    uint32_t get_readable_len() const 
    { 
        assert(_wpos >= _rpos);
        return _wpos - _rpos; 
    }


    char* get_writable_buff()
    {
        return _buf + _wpos;
    }

    uint32_t get_writable_len() const
    {
        assert(BUFSIZE >= _wpos);
        return BUFSIZE - _wpos;
        return 0;
    }

    //消费数据,不检验参数，请保证参数有效
    //@return 被消费的包头,用于上层读取报信息
    const pkg_head_t* read_package(size_t len) 
    { 
        void *data = _buf + _rpos;
        assert(get_readable_len() >= len);
        _rpos += len;
        assert(_rpos <= get_capacity());
        if (_rpos == _wpos) 
            _rpos = _wpos = 0;
        return (pkg_head_t*)data;
    }


    //如果头不足返回nullptr
    const pkg_head_t* peek_package_head() const
    {
        if (this->get_readable_len() < PKG_HEAD_LEN)
        {
            return nullptr;
        }
        return (pkg_head_t*)(_buf + _rpos);
    }


    //清除已读取的数据，返回清理的空间大小
    uint32_t discard_readed_data()
    {
       //TODO 策略
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


    //重置buffer状态为空
    inline void reset()
    {
        _rpos=0;
        _wpos = 0;
    }

    //检测buffer是否已经被破坏
    bool is_wrire_out_bound() const
    {
        return _cookie1 != COOKIE or _cookie2 != COOKIE;
    }


    //是否存在一个完成的包数据,传入包的长度
    bool is_complete_pkg(const size_t pkglen) const
    {
        return get_readable_len() >= pkglen;
    }


    void increase_write_len(size_t len)
    {
      _wpos += len;
      assert(_wpos <= BUFSIZE);
    }

    void update_recved_data(size_t datalen)
    {
        increase_write_len(datalen);
    }
};


template <typename pkg_head_t, size_t BUFSIZE, size_t PKG_HEAD_LEN=sizeof(pkg_head_t)>
struct net_sendbuf_t
{

  //
  //                                _hpos
  //                                 /  
  //                  | sendableLen  | pkglen| 
  //   +---+----------+--------------+-------+--------------+
  //   |   |sendedData| readableData         | writableLen |
  //   +---+----------+--------------+-------+--------------+
  //              /                         /              /
  //      _rpos                      _wpos     BUFSIZE 最大长度
  // 头部预留PKG_HEAD_LEN长度的空间,利于打包
  // _rpos 已发送数据的下一个
  // _hpos  指向正在写入的数据的包头，_hpos指向的数据不会被发送 
  // 如果打包后空间无容纳新的包头，则_hpos= _wpos,这时不可写，也没有包
  // 可用is_has_pack
  // _wpos 指向下一个将写入的地址
  //   

  enum {COOKIE=0xcccccccc};
  uint32_t _cookie1 = COOKIE;
  uint32_t _rpos        =0;
  uint32_t _hpos        =0;	
  uint32_t _wpos        =PKG_HEAD_LEN;	//预留一个包头
  char     _buf[BUFSIZE];
  uint32_t _cookie2 = COOKIE;

  //重置buffer为空
  void reset()
  {
      _rpos=0;
      _hpos=0;
      _wpos = PKG_HEAD_LEN;
  }

  size_t get_capacity() const
  {
      return BUFSIZE;
  }

  //uint32_t get_pkg_head_len() const
  //{
  //    return PKG_HEAD_LEN;
  //}

  //返回是否有正在打包的数据
  bool is_has_pkg() const
  {
      return _hpos != _wpos;
  }

  bool is_wrire_out_bound() const
  {
    return _cookie1 != COOKIE or _cookie2 != COOKIE;
  }


  uint32_t get_pkg_len() const
  {
    assert(_wpos >= _hpos);
    return _wpos - _hpos;
  }

  uint32_t get_pkg_body_len() const
  {
      if (_hpos == _wpos)
          return 0; //无空间容纳新包

      assert(_wpos >= _hpos + PKG_HEAD_LEN);
      return _wpos - _hpos - PKG_HEAD_LEN;
  }

  bool is_empty_pkg_body() const
  {
      return _wpos - _hpos == PKG_HEAD_LEN;
  }

  //返回已经发送完成的数据长度
  size_t get_sended_len() const {
      return _rpos;
  }

  //返回已经打包完成的可发送数据长度
  uint32_t get_sendable_len() const
  {
    assert(_hpos >= _rpos);
    return (_hpos - _rpos);
  }

  const char *get_sendable_data() const
  {
    return _buf + _rpos;
  }

  uint32_t get_writable_len() const
  {
      if (_wpos == _hpos)
          return 0;

      assert(_wpos <= BUFSIZE);
      return BUFSIZE - _wpos;
  }


  //当前必须有包,没有包返回nullptr
  template <class R=pkg_head_t>
  R* get_pkg_head()
  {
    assert(is_has_pkg());
    if (_wpos == _rpos)
        return nullptr;
    return (R*)(_buf + _hpos);
  }

  bool is_can_make_pkg() const
  {
      return is_has_pkg() and not is_empty_pkg_body();
  }


  void* get_pkg_body()
  {
    assert(is_has_pkg());
    if (_wpos == _rpos)
        return nullptr;
    return (void*)(_buf + _hpos + PKG_HEAD_LEN);
  }

  //并清理已经发送完成的数据,返回清理数据长度
  size_t discard_sended_data()
  {
    const uint32_t sendedLen = _rpos;
    if (sendedLen == 0) //unlikely
        return 0;

    memmove(_buf, _buf+_rpos, _wpos - _rpos);

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

  //检测是否有足够的空间写入指定长度数据,并会清理可用空间
  //@return true 可以写入,false空间不足无法写入
  bool ensure_writable_len(const size_t datalen)
  {
      if (get_writable_len() >= datalen)
          return true;

      if (get_writable_len() + get_sended_len() >= datalen) {
          discard_sended_data();
          assert(get_writable_len() >= datalen);
          return true;
      }

      return false;
  }


  //更新发送完数据，
  void update_sended_data(const size_t sendlen)
  {
    _rpos += sendlen;

    assert(_rpos <= _hpos);
    assert(_wpos <= BUFSIZE);

  }


  //丢弃到从上次make_next_pkg后写入的数据
  //返回丢弃的PkgBody长度
  uint32_t discard_pkg_data()
  {
      assert(is_has_pkg());
      uint32_t discardLen = _wpos - _hpos - PKG_HEAD_LEN;
      _wpos = _hpos + PKG_HEAD_LEN;
      return discardLen;
  }


  //调用后之前的数据将允许被发送,一般在打包后调用
  //预留一个PKG_HEAD_LEN长度空间，可以使用GetPkgHead返回包头指针
  //如果后续空间不足HEAD_LEN则设置_hpos = _wpos 
  bool make_next_pkg()
  {
    if (get_pkg_len() <= PKG_HEAD_LEN)
    {
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
  size_t append_value(T&& a)
  {
    static_assert(std::is_pod<std::remove_reference<T>>::value, "need POD type");
    static_assert(not std::is_array<std::remove_reference<T>>::value, "can not pass array type");
    static_assert(not std::is_pointer<std::remove_reference<T>>::value, "can not pass pointer type");
    //assert(GetWritableLen() >= sizeof(T));
    //printf("append &&T size :%lu\n", sizeof(T));

    *( typename std::remove_const<typename std::remove_reference<T>::type>::type *)(_buf + _wpos) = a;
    _wpos += sizeof(T);
    assert(_wpos <= BUFSIZE);
    return sizeof(T);
  }

  //不支持指针,数组，请使用AppendBuf
  template <typename T, size_t LEN> size_t append_value(T (&arr)[LEN]);
  template <typename T> size_t append_value(T* a);

  //支持可变参数 Append(3,54, 32L, obj) 参数需要是POD类型
  template<typename T, typename ...Args>
  size_t append_value(T&& a, Args... args)
  {
    return append_value(std::forward<T>(a)) + append_value(args...);
  }

  //写buffer数据,保证足够有写入长度
  size_t append_buffer(const void *data, const size_t dataLen)
  {
    assert(get_writable_len() >= dataLen);

    memcpy(_buf+_wpos, data, dataLen);
    _wpos += dataLen;
    assert(_wpos <= BUFSIZE);
    return dataLen;
  }

  //用于直接指针映射写入数据更新长度
  void increase_write_len(size_t len)
  {
      _wpos += len;
      assert(_wpos <= BUFSIZE);
  }

  char *get_writable_buff()
  {
      return _buf + _wpos;
  }
};



template <class subclass_t, class pkg_head_t, size_t RECVBUF_SIZE, size_t SENDBUF_SIZE=RECVBUF_SIZE>
struct net_fixbuff_t
{

   //返回能给个写入的长度
   std::pair<char*, size_t> get_recv_buff() 
   {
       _recvbuf.discard_readed_data();

       //如果buffsize 大于最大单个包长则不存full的情况

       return {
           _recvbuf.get_writable_buff(),
           _recvbuf.get_writable_len()
       };
   }

   void on_recv_complete(net_engine_t *eng, int sock, const char *data, const size_t datalen)
   {
       //接受的数据写入get_recv_buff 返回的空间,
       assert(datalen != 0);
       _recvbuf.update_recved_data(datalen);

        while (const pkg_head_t *head = _recvbuf.peek_package_head())
        {
            if (not head->is_valid_head())
            {
                //TODO 只检查一次
                static_cast<subclass_t*>(this)->on_recv_pkg_invalid(eng, sock, head);
                return;
            }

            if (not _recvbuf.is_complete_pkg(head->get_pkg_len()))
                return;

            _recvbuf.read_package(head->get_pkg_len()); //XXX 省略掉参数

            static_cast<subclass_t*>(this)->on_recv_pkg_complete(eng, sock, head, head->get_pkg_len()- sizeof(pkg_head_t));

            if (eng->is_skip_current_event_process())
                return;
        }
   }

   //返回需要发送的数据
   //自动实现
   std::pair<const char*, ssize_t> get_send_data()
   {
     return {
       _sendbuf.get_sendable_data(), _sendbuf.get_sendable_len()
     };
   }
 
   //自动实现
   void on_send_complete(net_engine_t *eng, int sock, const char *data, const ssize_t sendlen)
   {
     _sendbuf.update_sended_data(sendlen);

    //已发送数据超过buf长度的1/4后调整
    if (_sendbuf._rpos>= (SENDBUF_SIZE>>2))
        _sendbuf.discard_sended_data();

    if (_sendbuf.get_sendable_len() != 0)
        static_cast<subclass_t*>(this)->post_send(eng, sock);
   }


 protected:
   net_recvbuf_t<pkg_head_t, RECVBUF_SIZE> _recvbuf;
   net_sendbuf_t<pkg_head_t, SENDBUF_SIZE> _sendbuf;
};

/**
 * 组合IO层 + 收发buffer + 解包功能.
 * usage:
 * <code>
 * class BinClient : public tcp_active_conn_t<BinClient, pkg_head_t, _4K>  {
 * //需要实现如需函数
 * //connect callback
 * void on_conn_connecting(net_engine_t *eng, const char *ip, const uint16_t port, int sock)
 * void on_conn_complete(net_engine_t *eng, const char *ip, const uint16_t port, int sock)
 * void on_conn_failed(net_engine_t *eng, const char *ip, const uint16_t port, const int err)
 *
 * 
 * //当发现一个无效包时回调，应该关闭sock,如果不关闭行为未定义
 * void on_recv_pkg_invalid(net_engine_t *eng, int sock, const pkg_head_t *head)
 * void on_recv_pkg_complete(net_engine_t *eng, int sock, const pkg_head_t *pkg, const size_t bodylen)
 *
 * void on_peer_close(net_engine_t *eng, int sock)
 * void on_error_occur(net_engine_t *eng, int sock, int error)
 *
 * }
 * </code>
 */

template <class subclass_t, class pkg_head_t, size_t RECVBUF_SIZE, size_t SENDBUF_SIZE=RECVBUF_SIZE>
struct tcp_active_conn_t
: public async_tcp_client_hanndler_t<subclass_t>
, public net_fixbuff_t<subclass_t, pkg_head_t, RECVBUF_SIZE, SENDBUF_SIZE>
{};



/**
 * 组合IO层 + 收发buffer + 解包功能,方便业务层使用.
 * usage:
 * <code>
 * class BinServerConn: public tcp_passive_conn_t<BinServerConn, pkg_head_t, _4K>  {
 * //需要实现如需函数
 * //connect callback
 * 
 * //当发现一个无效包时回调，应该关闭sock,如果不关闭行为未定义
 * void on_recv_pkg_invalid(net_engine_t *eng, int sock, const pkg_head_t *head)
 * void on_recv_pkg_complete(net_engine_t *eng, int sock, const pkg_head_t *pkg, const size_t bodylen)
 *
 * void on_peer_close(net_engine_t *eng, int sock)
 * void on_error_occur(net_engine_t *eng, int sock, int error)
 *
 *  pkg_head_t 类型需要实现下面函数
 *  bool is_valid_head() const 
 *  uint32_t get_pkg_len() const 
 *
 * }
 * </code>
 */
template <class subclass_t, class pkg_head_t, size_t RECVBUF_SIZE, size_t SENDBUF_SIZE=RECVBUF_SIZE>
struct tcp_passive_conn_t 
  : public tcp_server_conn_handler_t<subclass_t>
  , public net_fixbuff_t<subclass_t, pkg_head_t, RECVBUF_SIZE, SENDBUF_SIZE>
{};

} //end namespace
