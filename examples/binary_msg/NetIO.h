
#ifndef NETIO_H_
#define NETIO_H_

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <assert.h>
#include "Utils.h"
#include "ASSERT.h"
#include "CommUtils.h"









//提供读写buff, 打包，接包，功能,配置Epoll使用
template <typename Derived, typename PkgHead, size_t RecvBuffSize, size_t SendBuffSize = RecvBuffSize>
struct CNetConnPkgBuf
{
    bool RecvPkgAndHandle(int fd)
    {
        if (not m_recvBuf.RecvPkg(fd))
        {
            return false;
        }
        while (const PkgHead *pkgHead = m_recvBuf.PeekNextPkgHead())
        {
            //解析数据，调用子类HandleNetPkg
            //Derived 需要实现 1. bool CheckIsPkgVaild(const NetPkgHaed *pkgHead),函数，
            //                  2. bool DispatchPkgMsg(const NetPkgHaed *pkgHead),函数，
            //使用CRTP,实现编译期重载
            if (not static_cast<Derived*>(this)->CheckIsPkgValid(pkgHead))
            {
                LOG_DBG("con fd:%u pkg invalid, will close it", fd);
                return false;
            }

            if (not m_recvBuf.IsHasCompletePkg(pkgHead->pkgLen))
            {
                //LOG_DBG("con fd:%u pkg not complete ",fd);
                return true;
            }

            m_recvBuf.ReadNextPkg(pkgHead->pkgLen);

            //TODO 传入body长度
            if (not static_cast<Derived*>(this)->HandleNetPkg(pkgHead, pkgHead->pkgLen - sizeof(PkgHead)))
            {
                return false;
            }
        }

        return true;
    }

  //发送可发送数据，用于处理EPOLLOUT事件
  int SendUnsendData(int fd, CEpoll *epoll)
  {
    int ret = m_sendBuf.SendPkg(fd);
    if (ret == -1)
    {
        LOG_ERR("SendBuf failed fd:%d %s will be close it", fd, strerror(errno));
        return -1;
    }

    if (m_sendBuf.GetSendableLen() == 0)
    {
        epoll->DisableOutEv(fd);
    }
    return ret;
  }


  NetPkgRecvBuf<PkgHead, RecvBuffSize> m_recvBuf;
  NetPkgSendBuf<PkgHead, SendBuffSize> m_sendBuf;
};


#endif
