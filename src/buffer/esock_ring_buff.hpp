#pragma once

#include "esock_buff_concept.hpp"

namespace esock {

//TODO PAGE_CNT to BUFSIZE
template <typename pkg_head_t, size_t BUFSIZE, size_t PKG_HEAD_LEN = sizeof (pkg_head_t)>
struct net_recv_ringbuf_t 
: IMPLEMENT_CONCEPT (ConnectRecvBuffer)
{
 private:
  enum {COOKIE=0xCCCCCCCC};

  uint32_t _cookie=COOKIE;
  uint32_t _write{};
  uint32_t _read{};
  char*    _buf{};
  uint32_t _bufsize{};

 public:
  /**
   * exception then construct faild
   * @throw-safe
   */
  net_recv_ringbuf_t() noexcept(false);
  ~net_recv_ringbuf_t();

 public:
  uint32_t inline wpos() const { return _write & (_bufsize -1); }
  uint32_t inline rpos() const { return _read & (_bufsize -1); }

 public:
    uint32_t get_capacity () const
    {
      return _bufsize;
    }

    uint32_t get_readable_len() const
    {
      return _write - _read;
    }

    char *get_writable_buff()
    {
      return _buf + wpos();
    }

    uint32_t get_writable_len() const
    {
      return 0;//todo
        //return _bufsize - write + read;
    }

    void increase_write_len(size_t len)
    {
        _write += len;
    }

    //清除已读取的数据，返回清理的空间大小
    uint32_t discard_readed_data ()
    {
      return 0;//do noting  可以都& mask
    }

    void reset ()
    {
      _write = 0;
      _read = 0;
    }


    bool is_full () const
    {
      return _bufsize == (_write - _read);
    }

    bool is_empty() const
    {
      return _write == _read;
    }

    bool is_complete_pkg (const size_t pkglen) const
    {
      return get_readable_len () >= pkglen;
    }

    bool is_wrire_out_bound () const
    {
      return false; //no support
    }

    const pkg_head_t *peek_package_head () const
    {
      return (pkg_head_t*)(_buf + rpos());
    }

    const pkg_head_t *read_package (size_t len)
    {
      pkg_head_t *data = _buf + rpos();
      _read += len;
      return data;
    }

}; //end class



template <typename pkg_head_t, size_t BUFSIZE, size_t PKG_HEAD_LEN = sizeof (pkg_head_t)>
struct net_send_ringbuf_t
: IMPLEMENT_CONCEPT (ConnectSendBuffer)
{
 private:
  enum {COOKIE=0xCCCCCCCC};

  uint32_t _cookie=COOKIE;
  uint32_t _wpos{};
  uint32_t _rpos{};
  char*    _buf{};
  //由于bufsize 的最低以为肯定不是0，
  //用bufsize的最低位来表示bufsize 是否为2的幂
  //如果是2的幂则 _bufsize = 实际大小-1
  uint32_t _bufsize{};

  inline uint32_t mod_size(uint32_t v) const
  {
    if (_bufsize & 0x1u)
      return v & _bufsize;
    else
      return v % _bufsize;
  }

 public:
  //@throw-safe
  net_send_ringbuf_t() noexcept(false);
  ~net_send_ringbuf_t();

    //Buff大小
    uint32_t get_capacity () const
    {
      if (_bufsize & 0x1u)
        return _bufsize + 1;
      else
        return _bufsize;
    }

    ////写入数据
    //template <class T, size_t LEN> size_t append_value (T (&arr)[LEN]);
    //template <class T> uint32_t append_value (T * a);
    //template <class T> uint32_t append_value (T && a);

    ////支持可变参数 Append(3,54, 32L, obj) 参数需要是POD类型
    //template <typename T, typename... Args> uint32_t append_value (T && a, Args... args);
    //uint32_t append_buffer (const void *data, const size_t dataLen);

    //压入一个值类型，不支持数组类型，指针类型以免出错
    //返回写入的字节数
    template <typename T> size_t append_value (T &&a)
    {
        static_assert (std::is_pod<std::remove_reference<T>>::value, "need POD type");
        static_assert (not std::is_array<std::remove_reference<T>>::value,
                       "can not pass array type");
        static_assert (not std::is_pointer<std::remove_reference<T>>::value,
                       "can not pass pointer type");

        *(typename std::remove_const<typename std::remove_reference<T>::type>::type *)(_buf + _wpos) = a;
        _wpos += sizeof (T);
        assert (_wpos <= get_capacity());
        _wpos = mod_size(_wpos);

        return sizeof (T);
    }


    //支持可变参数 Append(3,54, 32L, obj) 参数需要是POD类型
    template <typename T, typename... Args> size_t append_value (T &&a, Args... args)
    {
        return append_value (std::forward<T> (a)) + append_value (args...);
    }


    //不支持指针,数组，请使用AppendBuf
    template <typename T, size_t LEN> size_t append_value (T (&arr)[LEN]);
    template <typename T> size_t append_value (T *a);
    //写buffer数据,保证足够有写入长度
    size_t append_buffer (const void *data, const size_t dataLen)
    {
        assert (get_writable_len () >= dataLen);

        memcpy (_buf + _wpos, data, dataLen);
        _wpos += dataLen;
        assert (_wpos <= get_capacity());
        _wpos = mod_size(_wpos);
        return dataLen;
    }

    //丢弃到从上次make_next_pkg后写入的数据
    //返回丢弃的PkgBody长度
    uint32_t discard_pkg_data ()
    {
      return 0; //do nothing
    }

    //并清理已经发送完成的数据,返回清理数据长度
    size_t discard_sended_data ()
    {

    }


    //检测是否有足够的空间写入指定长度数据,并会清理可用空间
    //@return true 可以写入,false空间不足无法写入
    bool ensure_writable_len (const size_t datalen)
    {

    }


    //返回包头指针
    pkg_head_t *get_pkg_head ();

    //返回包体指针
    void *get_pkg_body ();

    //返回当前包体大小
    uint32_t get_pkg_body_len () const;

    //返回当前包大小 包头+包体
    uint32_t get_pkg_len () const;

    //返回未发送的数据指针
    const char *get_sendable_data () const;

    //返回已经打包完成的可发送数据长度
    uint32_t get_sendable_len () const;

    //更新发送完数据，
    void update_sended_data (const size_t sendlen);

    //返回已经发送完成的数据长度
    size_t get_sended_len () const;

    //返回可写地址
    char *get_writable_buff ();

    //返回可写入字节数
    uint32_t get_writable_len () const;

    //更新写入数据长度
    void increase_write_len (size_t len);

    //是否有空间构造新的包
    bool is_can_make_pkg () const;

    //构造一个新包
    bool make_next_pkg ();

    //当前buff是否有包
    bool is_has_pkg () const;

    //当前包体是否为空
    bool is_empty_pkg_body () const;

    //重置buffer
    void reset ();

    //是否写越界
    bool is_wrire_out_bound () const;

};

}// end namespace esock
