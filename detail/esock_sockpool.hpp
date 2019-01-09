/**
 * @file  
 *
 */

#pragma once

#include "../esock_socket.hpp"
#include "esock_utility.hpp"

namespace esock {

enum sockinfo_type_t {
  ESOCKTYPE_NONE=0,
  ESOCKTYPE_EPOLL=1,
  ESOCKTYPE_TCP_LISTENER=2,
  ESOCKTYPE_TCP_CONNECT=3
};

enum sockinfo_state_t {
  ESOCKSTATE_CLOSED = 0,
  ESOCKSTATE_CREATE = 1, //完成socket 创建
  ESOCKSTATE_LISTENING = 2, //调用listener
  ESOCKSTATE_CONNECTING= 3, //connect 返回EINPROGRESS 
  ESOCKSTATE_ESTABLISHED = 4,
  ESOCKSTATE_ERROR_OCCUES = 5, //有错误发生
};



class net_engine_t;
struct sockinfo_t
{
  uint8_t _instance:1; 
  uint8_t _is_in_epoll:1; 
  uint8_t _type;
  uint8_t _state;
  net_engine_t *_eng;
  //union {
  //  struct { //epoll
  //    epoll_t *_epoll;
  //  };
  //  struct { //tcp_server
  //    net_engine_t *_eng;
  //  };

  //};
  void * _on_recvable_fn;
  void * _on_sendable_fn;
  void *_arg;

 public:
  void close(int sock);

  void init(sockinfo_type_t type)
  {
    esock_assert(is_closed());
    _type = type;
    _state = ESOCKSTATE_CREATE;
    _is_in_epoll = 0;
    _on_recvable_fn = nullptr;
    _on_sendable_fn = nullptr;
    _arg = nullptr;
  }

  bool is_closed() const {
    return _state == ESOCKSTATE_CLOSED;
  }

  int get_fd() const;
};


struct sockpool_t : detail::noncopyable_t
{
 public:
  sockinfo_t *_socks{};
  size_t     _size{};


  sockinfo_t* get_info(int fd) {
    if (fd > _size) return nullptr;
    return _socks + fd;
  }

 private:
  int init();
  void uninit();

  friend class net_engine_t;
 private:
  int _ref=0;
  
};


extern sockpool_t sockpool;

inline int sockinfo_t::get_fd() const {return this - sockpool._socks;}
//static inline void close_socket(int fd) { sockpool.get_info(fd)->close(fd); }

} //namespace

