/**
 * @file  
 *
 */

#pragma once

#include <strings.h>
#include "../esock_socket.hpp"
#include "esock_utility.hpp"

namespace esock {

enum sockinfo_type_t {
  ESOCKTYPE_NONE=0,
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


//所有esock函数返回的socket，必须调用该函数进行关闭
int close_socket(int sock);

class net_engine_t;
struct sockinfo_t
{
  uint8_t _instance:1; 
  uint8_t _is_in_epoll:1; 
  uint8_t _is_set_epollin:1;
  uint8_t _is_set_epollout:1;
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
    ::bzero(this, sizeof(*this));
    _type = type;
    _state = ESOCKSTATE_CREATE;
  }

  bool is_closed() const {
    return _state == ESOCKSTATE_CLOSED;
  }

  int get_fd() const;
};


class sockpool_t : detail::noncopyable_t
{

 private:
  int init();
  void uninit();

 private:
  sockinfo_t *_socks{};
  int         _size{};

 public:
  void clear_epoll_pointer(const net_engine_t *eng);

  sockinfo_t* get_info(int fd) {
    if (fd > _size) return nullptr;
    return _socks + fd;
  }

  friend class sockinfo_t;
  friend class net_engine_t;

 private:
  volatile int _ref=0;
  
};

extern sockpool_t sockpool; //gloabl varibale

//static inline void close_socket(int fd) { sockpool.get_info(fd)->close(fd); }

} //namespace

