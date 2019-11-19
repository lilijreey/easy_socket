## Design pinciple
top
1. 简单易用
2. 最小约束
3. 正交设计可组合
4. 零开销抽象
5. 分层
6. 灵活


## 特性
* delete this safe

## 限制
只适用于X86-64体系下的Linux系统
暂时其他系统和体系环境不支持


## How to user


## Install
1. make
2. make install
3. -lesock

## Compile

业务逻辑
--------------------
connect handler-业务call_back
--------------------
   buffer 
----------------------
connect handler
-----------------------
engine
-----------------------
kernel

组件
===========================
1. engine
   网络事件引擎,从内核读取/写入数据到buffer
   

2. net handler
   作为engine和buffer的交互层,隔离了engine和buffer的直接耦合交互

   所有Handler公用一组统一接口
   TcpConnectHan


3. buffer 
    接口分为RecvBuffer 和SendBuffer
    * RecvBuffer 
      用于实现接收网络数据,并提供给应用层读取包括拆包功能

    * SendBuffer
      用于实现发送数据缓存




