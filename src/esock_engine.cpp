/**
 * @file     esock_engine.cpp
 *
 *
 * @author   lili  <lilijreey@gmail.com>
 * @date     01/05/2019 02:09:04 PM
 *
 */

#include <arpa/inet.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "detail/esock_sockpool.hpp"
#include "esock_engine.hpp"

namespace esock
{

net_engine_t *net_engine_t::make ()
{
    net_engine_t *eng = new (std::nothrow) net_engine_t ();
    if (eng == nullptr) return nullptr;

    if (eng->init () == -1)
    {
        delete eng;
        return nullptr;
    }

    return eng;
}

void net_engine_t::unmake (net_engine_t *&eng)
{
    delete eng;
    eng = nullptr;
}

int net_engine_t::init ()
{
    esock_assert (_efd == -1);

    if (sockpool.init () != 0) return -1;

    _efd = ::epoll_create1 (0);
    return _efd;
}

// uninit
net_engine_t::~net_engine_t ()
{
    _is_stop = true;
    if (_efd != -1)
    {
        ::close (_efd);
        _efd = -1;
        sockpool.clear_epoll_pointer (this);
        sockpool.uninit ();
    }
}

// static async_process

void net_engine_t::async_tcp_connect (const std::string &ip,
                                      const uint16_t port,
                                      on_tcp_conn_complete_fn_t on_conn_complete_fn,
                                      on_tcp_conn_failed_fn_t on_conn_failed_fn,
                                      void *user_arg)
{
    // sockaddr_storage addr;
    // TODO 抽象成一个函数 build_sock_addr
    int sock = -1;
    sockinfo_t *sinfo = nullptr;
    sockaddr_in addr;

    if (not init_sockaddr_in (addr, ip, port))
    {
        esock_report_error_msg ("an invalid ip %s", ip.c_str ());
        goto error;
    }

    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (-1 == sock)
    {
        esock_report_syserr_msg ("create socket failed %d ", 3);
        goto error;
    }

    // sinfo init
    sinfo = sockpool.get_info (sock);
    sinfo->init (ESOCKTYPE_TCP_CONNECT); // TODO init 和socket内聚到一个函数中

    if (-1 == set_sock_nonblocking (sock))
    {
        esock_report_syserr_msg ("set socket %d nonblock failed", sock);
        goto error;
    }

    if (-1 == ::connect (sock, (sockaddr *)&addr, sizeof (addr)))
    {
        if (EINPROGRESS == errno)
        {
            sinfo->_state = ESOCKSTATE_CONNECTING;
            if (-1 == epoll_add_sock (sock, EPOLLOUT, (void *)on_conn_failed_fn, (void *)on_conn_complete_fn, user_arg))
            {
                goto error;
            }

            // not real failed
            on_conn_failed_fn (this, sock, EINPROGRESS, user_arg);
            return;
        }

        esock_report_syserr_msg ("connect failed fd:%d", sock);
        goto error;
    }

    on_conn_complete_fn (this, sock, user_arg);
    return;

<<<<<<< HEAD
error:
    if (sock != -1) close_socket (sock);
    on_conn_failed_fn (this, -1, errno, user_arg);
=======
int net_engine_t::epoll_add_sock(int sock,
                    int events, 
                    void *on_recvable,
                    void *on_sendable,
                    void *arg)
{
  sockinfo_t *sinfo = sockpool.get_info(sock);
  if (sinfo->is_closed()) {
    esock_set_error_msg("sock %d is closed", sock);
    errno = EINVAL;
    return -1;
  }

  if (sinfo->_eng == this && sinfo->_is_in_epoll) {
    esock_set_error_msg("sock %d already add to eng", sock);
    errno = EEXIST;
    return -1;
  }


  epoll_event ev;
  //ev.events = events ;// | EPOLLET;
  //ET 模式有很多在man中没有写明的行为
  ev.events = events | EPOLLET;
  ev.data.ptr = (void*)((uintptr_t)sinfo | !sinfo->_instance);

  int ret = epoll_ctl(_efd, EPOLL_CTL_ADD, sock, &ev);
  if (ret == -1)
  {
    esock_set_syserr_msg("epoll_ctl add fd %d failed", sock);
    return -1;
  };

  sinfo->_eng = this;
  sinfo->_is_in_epoll = 1;
  sinfo->_on_recvable_fn = on_recvable;
  sinfo->_on_sendable_fn = on_sendable;
  sinfo->_arg = arg;

  //后面添加instance 用来表示fd是否已过期,参考nginx
  sinfo->_instance = !sinfo->_instance;

  esock_debug_log("add fd:%d in epoll ok\n", sock);
  return 0;
>>>>>>> 0cc55b20ffef6473b8410c1899d2558dabfe174c
}

int net_engine_t::epoll_add_sock (int sock, int events, void *on_recvable, void *on_sendable, void *arg)
{
<<<<<<< HEAD
    sockinfo_t *sinfo = sockpool.get_info (sock);
    if (sinfo->is_closed ())
    {
        esock_report_error_msg ("sock %d is closed", sock);
        errno = EINVAL;
        return -1;
=======
  if (is_stop())
  {
    return 0;
  }

  const int ev_cnt = epoll_wait(_efd, _events, MAX_EVENT_SIZE, wait_event_ms.count());
  if (ev_cnt == -1)
  {
    esock_set_syserr_msg("epoll_wait fd:%d failed", _efd);
    if (errno == EINTR)
      return 0;
    return -1;
  }

  sockinfo_t *sinfo = nullptr;
  for (int i =0; i < ev_cnt; ++i)
  {
    //批处理时需要正确处理  see epoll(7)
    //1.Starvation (ET)
    //2.event cache 
    
    void *p = _events[i].data.ptr;
    int events = _events[i].events;

    int instance = (uintptr_t) p & 1;
    sinfo = (sockinfo_t*) ((uintptr_t) p & (uintptr_t) ~1);

    const int fd = sinfo->get_fd();

    _current_fd = fd;
    _is_skip_current_event_process =false;

    esock_debug_log("fd %d has ev :%s %s %s\n", fd, events & EPOLLIN ? "EPOLLIN": "", events&EPOLLOUT ? "EPOLLOUT" : "", events&EPOLLERR?"EPOLLERR":"");

    if (sinfo->is_closed()) {
      esock_debug_log("process sock %d event but is closed, skip", fd);
      continue;
>>>>>>> 0cc55b20ffef6473b8410c1899d2558dabfe174c
    }

    if (sinfo->_eng == this && sinfo->_is_in_epoll)
    {
        esock_report_error_msg ("sock %d already add to eng", sock);
        errno = EEXIST;
        return -1;
    }

    epoll_event ev;
    ev.events = events & ~EPOLLET; // remove ET if set
    ev.data.ptr = (void *)((uintptr_t)sinfo | !sinfo->_instance);

    int ret = epoll_ctl (_efd, EPOLL_CTL_ADD, sock, &ev);
    if (ret == -1)
    {
        esock_report_syserr_msg ("epoll_ctl add fd %d failed", sock);
        return -1;
    };

    sinfo->_eng = this;
    sinfo->_is_in_epoll = 1;
    sinfo->_is_set_epollin = events & EPOLLIN;
    sinfo->_is_set_epollout = events & EPOLLOUT;
    sinfo->_on_recvable_fn = on_recvable;
    sinfo->_on_sendable_fn = on_sendable;
    sinfo->_arg = arg;

    //后面添加instance 用来表示fd是否已过期,参考nginx
    sinfo->_instance = !sinfo->_instance;

    esock_debug_log ("add fd:%d in epoll ok\n", sock);
    return 0;
}

int net_engine_t::process_event (std::chrono::milliseconds wait_event_ms)
{
    if (is_stop ())
    {
        return 0;
    }

    const int ev_cnt = epoll_wait (_efd, _events, MAX_EVENT_SIZE, wait_event_ms.count ());
    if (ev_cnt == -1)
    {
        esock_report_syserr_msg ("epoll_wait fd:%d failed", _efd);
        if (errno == EINTR) return 0;
        return -1;
    }

    sockinfo_t *sinfo = nullptr;
    for (int i = 0; i < ev_cnt; ++i)
    {
        //批处理时需要正确处理  see epoll(7)
        // 1.Starvation (ET)
        // 2.event cache

        void *p = _events[i].data.ptr;
        int events = _events[i].events;

        int instance = (uintptr_t)p & 1;
        sinfo = (sockinfo_t *)((uintptr_t)p & (uintptr_t)~1);

        const int fd = sinfo->get_fd ();

        _current_fd = fd;
        _is_skip_current_event_process = false;

        esock_debug_log ("fd %d has ev\n", fd);

        if (sinfo->is_closed ())
        {
            esock_debug_log ("process sock %d event but is closed, skip", fd);
            continue;
        }

        // TODO test
        if (sinfo->_instance != instance)
        {
            esock_debug_log ("process sock %d event but is not same instance, skip", fd);
            continue;
        }

        if (sinfo->_eng != this)
        {
            esock_debug_log ("process sock %d event but already remove from epoll, skip", fd);
            continue;
        }

        //当EPOLLERR出现时，调用read 可以发现错，
        // EPOLLHUP 调用read返回0
        if (events & (EPOLLERR | EPOLLHUP)) events |= (EPOLLIN | EPOLLOUT);

        if (events & EPOLLIN)
        {
            switch (sinfo->_type)
            {
            case ESOCKTYPE_NONE:
                esock_assert (false);
                close_socket (fd);
                continue;
            case ESOCKTYPE_TCP_LISTENER:
                ((on_tcp_listener_acceptable_fn_t)sinfo->_on_recvable_fn) (
                this, static_cast<tcp_listener_t *> (sinfo->_on_sendable_fn), fd, sinfo->_arg);
                continue;

            case ESOCKTYPE_TCP_CONNECT:
                if (sinfo->_state == ESOCKSTATE_CONNECTING)
                {
                    // error
                    sinfo->_state = ESOCKSTATE_ERROR_OCCUES;
                    int error = get_sock_error (fd);
                    ((on_tcp_conn_failed_fn_t)sinfo->_on_recvable_fn) (this, fd, error, sinfo->_arg);
                    close_socket (fd);
                    continue;
                }

                ((on_tcp_conn_recvable_fn_t)sinfo->_on_recvable_fn) (this, fd, 0, sinfo->_arg);
                break;

            default:
                esock_assert (false);
                close_socket (fd);
                continue;
            }
        } // EPOLLIN

        if (not sinfo->_is_in_epoll || sinfo->_eng != this) continue;

        if (events & EPOLLOUT) switch (sinfo->_type)
            {
            case ESOCKTYPE_TCP_CONNECT:
                if (sinfo->_state == ESOCKSTATE_CONNECTING)
                {
                    sinfo->_state = ESOCKSTATE_ESTABLISHED;
                    epoll_del_sock (sinfo);
                    ((on_tcp_conn_complete_fn_t)sinfo->_on_sendable_fn) (this, fd, sinfo->_arg);
                    continue;
                }

                ((on_tcp_conn_sendable_fn_t)sinfo->_on_sendable_fn) (this, fd, 0, sinfo->_arg);
                break;

            default:
                esock_assert (false);
            }

    } // end for

    _current_fd = -1;
    _is_skip_current_event_process = false;

    return ev_cnt;
}

void net_engine_t::epoll_del_sock (sockinfo_t *sinfo)
{
    esock_assert (sinfo->_is_in_epoll && sinfo->_eng == this);
    const int fd = sinfo->get_fd ();

    epoll_event _ev; // 2.6.9 before need
    if (-1 == epoll_ctl (_efd, EPOLL_CTL_DEL, fd, &_ev))
        esock_report_syserr_msg ("epoll_ctl del fd %d failed\n", fd);

    sinfo->_is_in_epoll = 0;
    sinfo->_is_set_epollin = 0;
    sinfo->_is_set_epollout = 0;

    if (_current_fd == fd) _is_skip_current_event_process = true;

    esock_debug_log ("del fd %d ok\n", fd);
}

// int net_engine_t::close_socket(sockinfo_t *sinfo)
//{
//  sinfo->close();
//  return 0;
//}

int close_socket (int fd)
{
    sockinfo_t *sinfo = sockpool.get_info (fd);
    if (sinfo == nullptr)
    {
        esock_report_error_msg ("a invilded fd %d", fd);
        return -1;
    }
    sinfo->close (fd);
    return 0;
}

int net_engine_t::close_socket (int fd)
{
    sockinfo_t *sinfo = sockpool.get_info (fd);
    if (sinfo == nullptr)
    {
        esock_report_error_msg ("a invilded fd %d", fd);
        return -1;
    }
    sinfo->close (fd);
    return 0;
}

int net_engine_t::add_tcp_listener (tcp_listener_t *listener,
                                    on_tcp_listener_acceptable_fn_t on_acceptable_fn,
                                    void *arg)
{
    const int listen_fd = listener->get_sock ();
    if (listen_fd == -1)
    {
        esock_report_syserr_msg ("listener not open");
        return -EINVAL;
    }

    sockinfo_t *sinfo = sockpool.get_info (listen_fd);
    esock_assert (sinfo->_type == ESOCKTYPE_TCP_LISTENER);

    if (sinfo->_eng != nullptr)
    {
        esock_report_error_msg ("alreay add engine");
        return -EINVAL;
    }

    esock_assert (not sinfo->_is_in_epoll);

    return epoll_add_sock (listen_fd, EPOLLIN, (void *)on_acceptable_fn, (void *)listener, arg);
}

int net_engine_t::add_sendable_ev (int sock)
{
    sockinfo_t *sinfo = sockpool.get_info (sock);

    if (sinfo->_is_set_epollout)
    {
        esock_debug_log ("fd already set epollout");
        return 0;
    }
    esock_assert (not sinfo->is_closed ());
    esock_assert (sinfo->_eng == this);

    epoll_event ev;
    if (sinfo->_is_set_epollin)
        ev.events = EPOLLIN | EPOLLOUT;
    else
        ev.events = EPOLLOUT;

    ev.data.ptr = (void *)((uintptr_t)sinfo | sinfo->_instance); //取饭还原之前的

    int ret = epoll_ctl (_efd, EPOLL_CTL_MOD, sock, &ev);
    if (ret == -1)
    {
        esock_report_syserr_msg ("epoll_ctl_mod fd %d failed", sock);
        return -1;
    };

    sinfo->_is_set_epollout = true;

    esock_debug_log ("add sendable fd:%d in epoll ok\n", sock);
    return 0;
}

int net_engine_t::del_sendable_ev (int sock)
{
    sockinfo_t *sinfo = sockpool.get_info (sock);

    esock_assert (not sinfo->is_closed ());
    esock_assert (sinfo->_eng == this);
    esock_assert (sinfo->_is_set_epollin);
    esock_assert (sinfo->_is_set_epollout);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = (void *)((uintptr_t)sinfo | sinfo->_instance); //取饭还原之前的

    int ret = epoll_ctl (_efd, EPOLL_CTL_MOD, sock, &ev);
    if (ret == -1)
    {
        esock_report_syserr_msg ("epoll_ctl_mod fd %d failed", sock);
        return -1;
    };

    sinfo->_is_set_epollout = false;

    esock_debug_log ("del sendable fd:%d in epoll ok\n", sock);
    return 0;
}

} // end namespace esock
