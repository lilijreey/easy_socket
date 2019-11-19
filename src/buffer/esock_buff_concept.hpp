#pragma once

#include "../detail/esock_concept.hpp"

namespace esock
{

//TODO concept
//实现新的buffer需要实如下接口
DEF_CONCEPT (ConnectRecvBuffer)
//:detail::noncopyable_t
{
    struct pkg_head_t;

    //返回buffer的大小
    uint32_t get_capacity () const;

    //返回可以读取的数据长度
    uint32_t get_readable_len () const;

    //重置buffer
    void reset ();

    //返回可写数据长度
    uint32_t get_writable_len () const;

    //write_data
    //返回可写地址,用于recv直接写入,少一次copy
    char *get_writable_buff ();

    //更新buffer的写入数据长度,由写入逻辑更新
    void increase_write_len (size_t len);

    //清除已读取的数据，返回清理的空间大小
    uint32_t discard_readed_data ();

    bool is_full () const;
    bool is_empty() const;

    //可读数据长度是否大于等于pkglen
    bool is_complete_pkg (const size_t pkglen) const;

    //是否写越界
    bool is_wrire_out_bound () const;

    //返回可读取数据头地址
    const pkg_head_t *peek_package_head () const;

    //消费数据,不检验参数，请保证参数有效
    //@return 被消费的包头,用于上层读取报信息
    const pkg_head_t *read_package (size_t len);
};

DEF_CONCEPT (ConnectSendBuffer)
//:detail::noncopyable_t
{
    struct pkg_head_t;

    //写入数据
    template <class T, size_t LEN> size_t append_value (T (&arr)[LEN]);
    template <class T> uint32_t append_value (T * a);
    template <class T> uint32_t append_value (T && a);

    //支持可变参数 Append(3,54, 32L, obj) 参数需要是POD类型
    template <typename T, typename... Args> uint32_t append_value (T && a, Args... args);
    uint32_t append_buffer (const void *data, const size_t dataLen);

    //丢弃到从上次make_next_pkg后写入的数据
    //返回丢弃的PkgBody长度
    uint32_t discard_pkg_data ();

    //并清理已经发送完成的数据,返回清理数据长度
    size_t discard_sended_data ();

    //检测是否有足够的空间写入指定长度数据,并会清理可用空间
    //@return true 可以写入,false空间不足无法写入
    bool ensure_writable_len (const size_t datalen);

    //Buff大小
    uint32_t get_capacity () const;

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

} // end namespace
