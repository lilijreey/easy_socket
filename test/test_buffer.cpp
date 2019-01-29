

#include <gtest/gtest.h>
#include <errno.h>
#include "../src/esock.hpp"

#define protected public
#define private public


using namespace esock;

constexpr size_t _1K = 1 << 10;
constexpr size_t _2K = 2 << 10;
constexpr size_t _3K = 3 << 10;
constexpr size_t _4K = 4 << 10;


TEST(ESOCK, TestNetRecvBuf)
{
  struct pkg_head_t 
  {
    uint32_t len;
    uint32_t cmdid;
    char data[0];
  };

  pkg_head_t *pkg = (pkg_head_t*)malloc(_4K);
  pkg->len=_4K;
  pkg->cmdid = 3;

  using recvbuf_t = net_recvbuf_t<pkg_head_t,_4K>;
  recvbuf_t buf;
  ASSERT_EQ(buf.get_capacity(), _4K);
  memcpy(buf.get_writable_buff(), pkg, 64);
  buf.increase_write_len(_4K);
  ASSERT_EQ(_4K, buf._wpos);
  ASSERT_EQ(0, buf.get_writable_len());
  ASSERT_EQ(_4K, buf.get_readable_len());
  auto peekPkg =  buf.peek_package_head();
  ASSERT_EQ(_4K, peekPkg->len);
  ASSERT_EQ(3, peekPkg->cmdid);

  ASSERT_TRUE(buf.is_complete_pkg(_4K));

  //read buf
  auto readPkg = buf.read_package(_4K);
  ASSERT_EQ(readPkg, peekPkg);
  ASSERT_EQ(_4K, readPkg->len);
  ASSERT_EQ(3, readPkg->cmdid);
  //empty buf
  ASSERT_EQ(0, buf.get_readable_len()); 

  buf.discard_readed_data();

  //test discard_readed_data
  buf.reset();
  ASSERT_TRUE(buf.get_readable_len() == 0);

  buf.increase_write_len(_1K);
  ASSERT_EQ(buf.get_readable_len(), _1K);
  int to = 0;
  for (int i=0; i<10; ++i)
  {
      buf.read_package(10*i);
      to += 10 * i;
      ASSERT_EQ(buf.get_readable_len(), _1K-to);
  }

  ASSERT_EQ(to, buf.discard_readed_data());
  ASSERT_EQ(buf.get_readable_len(), _1K-to);
  ASSERT_EQ(0, buf.discard_readed_data());

  buf.read_package(_1K-to);
  ASSERT_EQ(buf.get_readable_len(), 0);
  //ASSERT_EQ(_1K-to, buf.discard_readed_data());
  //ASSERT_EQ(0, buf.discard_readed_data());

  buf.reset();
  ASSERT_EQ(buf.get_readable_len(), 0);
  buf.increase_write_len(_1K);
  ASSERT_EQ(buf.get_readable_len(), _1K);
  buf.increase_write_len(_1K);
  ASSERT_EQ(buf.get_readable_len(), _2K);
  buf.increase_write_len(_1K);
  ASSERT_EQ(buf.get_readable_len(), _3K);
  buf.increase_write_len(_1K-1);
  ASSERT_EQ(buf.get_readable_len(), _4K-1);
  ASSERT_EQ(buf.get_writable_len(), 1);
  ASSERT_EQ(buf.discard_readed_data(), 0);
  ASSERT_EQ(buf.get_writable_len(), 1);

  buf.read_package(1);
  ASSERT_EQ(buf.get_readable_len(), _4K-2);
  ASSERT_EQ(buf.get_writable_len(), 1);
  ASSERT_EQ(buf.discard_readed_data(), 1);
  ASSERT_EQ(buf.get_writable_len(), 2);

  buf.read_package(0);
  ASSERT_EQ(buf.get_readable_len(), _4K-2);;
  ASSERT_EQ(buf.get_writable_len(), 2);
  ASSERT_EQ(buf.discard_readed_data(), 0);
  ASSERT_EQ(buf.get_writable_len(), 2);

  buf.increase_write_len(1);
  ASSERT_EQ(buf.get_readable_len(), _4K-1);
  ASSERT_EQ(buf.get_writable_len(), 1);
  ASSERT_EQ(buf.discard_readed_data(), 0);
  ASSERT_EQ(buf.get_writable_len(), 1);

  buf.increase_write_len(1);
  ASSERT_EQ(buf.get_readable_len(), _4K);
  ASSERT_EQ(buf.get_writable_len(), 0);
  ASSERT_EQ(buf.discard_readed_data(), 0);
  ASSERT_EQ(buf.get_writable_len(), 0);

  buf.read_package(_4K);
  ASSERT_EQ(buf.get_readable_len(), 0);
  ASSERT_EQ(buf.discard_readed_data(), 0);
}


#if 0
TEST(NetIO, TestNetPkgSendBuf)
{

  struct pkg_head_t 
  {
    uint32_t len;
    uint32_t cmdid;
    char data[0];
  };

  pkg_head_t *pkg = (pkg_head_t*)malloc(_1K);
  pkg->len=_1K;
  pkg->cmdid = 2012;
  memcpy(pkg->data, "123456",6);

  using SendBuf = NetPkgSendBuf<pkg_head_t,_4K>;
  SendBuf buf;

  ASSERT_EQ(buf.GetSendableLen() , 0);
  ASSERT_TRUE(buf.IsEmptyPkgBody());
  ASSERT_EQ(buf.GetPkgBodyLen(), 0);
  ASSERT_EQ(buf.get_writable_len(), buf.GetCapacity() - buf.Getpkg_head_tLen());
  ASSERT_EQ(buf.GetPkgLen(), buf.Getpkg_head_tLen());

  //Append 
  char i8=5;
  int16_t i16=16;
  int32_t i32=32;
  uint64_t i64=3;
  int * pi = (int*)1;

  size_t writeLen = 1+2+4+8;
  ASSERT_EQ(buf.AppendValue(i8), sizeof(i8));
  ASSERT_EQ(buf.AppendValue(i16), sizeof(i16));
  ASSERT_EQ(buf.AppendValue(i32), sizeof(i32));
  ASSERT_EQ(buf.AppendValue(i64), sizeof(i64));
  ASSERT_EQ(buf.GetPkgBodyLen(),  writeLen);
  ASSERT_EQ(buf.get_writable_len(),  buf.GetCapacity()- buf.GetPkgLen());

#define BodyOffset(buf, n) ((buf.Getpkg_head_t()->data) + n)

  ASSERT_EQ(i8,   *(char*)BodyOffset(buf,0));
  ASSERT_EQ(i16, *(int16_t*)BodyOffset(buf, 1));
  ASSERT_EQ(i32, *(int32_t*)BodyOffset(buf,3));
  ASSERT_EQ(i64, *(int64_t*)BodyOffset(buf,7));

  buf.Reset();
  ASSERT_EQ(buf.GetSendableLen() , 0);
  ASSERT_TRUE(buf.IsEmptyPkgBody());
  ASSERT_EQ(buf.GetPkgBodyLen(), 0);
  ASSERT_EQ(buf.get_writable_len(), buf.GetCapacity() - buf.Getpkg_head_tLen());
  ASSERT_EQ(buf.GetPkgLen(), buf.Getpkg_head_tLen());
  //检测内容
  

  char str[] ="12345";
  buf.AppendBuffer(str, 5);
  ASSERT_EQ(buf.GetPkgBodyLen(), 5);
  ASSERT_EQ(strcmp((char*)buf.GetPkgBody(), str), 0);

  const char cstr[] ="12345";
  buf.AppendBuffer(str,5);
  ASSERT_EQ(buf.GetPkgBodyLen(), 10);
  for (int i=0; i < 5; ++i)
  {
      ASSERT_EQ(((char*)buf.GetPkgBody())[i], i+1+'0');
  }
  ASSERT_EQ(buf.AppendValue((uint8_t)'9'), sizeof(uint8_t));
  ASSERT_EQ(buf.AppendValue((uint8_t)'8'), sizeof(uint8_t));
  ASSERT_EQ(buf.AppendValue((uint8_t)'7'), sizeof(uint8_t));
  ASSERT_EQ(buf.GetPkgBodyLen(), 13);
  ASSERT_EQ(strcmp((char*)buf.GetPkgBody(), "1234512345987"), 0);

  //写入字面值
  buf.Reset();
  ASSERT_EQ(buf.AppendValue((uint8_t)1), sizeof(uint8_t));
  ASSERT_EQ(buf.AppendValue((uint16_t)2), sizeof(uint16_t));
  ASSERT_EQ(buf.AppendValue((uint64_t)3), sizeof(uint64_t));
  ASSERT_EQ(1,   *(uint8_t*)BodyOffset(buf,0));
  ASSERT_EQ(2,   *(uint16_t*)BodyOffset(buf,1));
  ASSERT_EQ(3,   *(uint64_t*)BodyOffset(buf,3));

  //写入自定义类型
  struct C1
  {
      int a;
      int b;
      int c;
  };
  C1 c1{1,2,3};
  buf.Reset();
  ASSERT_EQ(buf.AppendValue(c1), sizeof(c1));
  C1 *pC1 = (C1*)buf.GetPkgBody();
  ASSERT_EQ(pC1->a, c1.a);
  ASSERT_EQ(pC1->b, c1.b);
  ASSERT_EQ(pC1->c, c1.c);
  ASSERT_EQ(buf.GetSendableLen(), 0);

  //混合写
  ASSERT_EQ(buf.AppendValue(0,0,0, c1), 4+4+4+sizeof(c1));

  char *data1K = (char*)malloc(_1K);
  bzero(data1K, _1K);
  ASSERT_EQ(buf.AppendBuffer(data1K, _1K), _1K);

  auto pkgLen = buf.GetPkgLen();
  buf.MakeNextPkg();
  ASSERT_EQ(buf.GetPkgBodyLen(), 0);
  ASSERT_EQ(buf.GetSendableLen(), pkgLen);

  //append enum
  enum {
      ONE, TWO
  };

  ASSERT_EQ(buf.AppendValue((uint32_t)ONE), 4);
  ASSERT_EQ(buf.AppendValue((uint8_t)ONE), 1);
  ASSERT_EQ(buf.AppendValue((uint16_t)ONE), 2);
  ASSERT_EQ(buf.AppendValue(ONE, TWO), 8);


  char data[100];
  int dataLen=12;
  ASSERT_EQ(12 , buf.AppendBuffer(data, dataLen));
  //ASSERT_EQ(12 , buf.AppendValue(data));
  //ASSERT_EQ(12 , buf.AppendValue(data)); //编译错误


struct MatchInfo
{
    uint64_t infoKey;
    uint16_t port; //listen port
    uint8_t attr; 

}__attribute__((packed));

MatchInfo info = {11,22,3};



  MatchInfo *po= (MatchInfo*)(buf.m_buf + buf.m_writePos);
  ASSERT_EQ(buf.AppendValue(i64, i16, info.attr), sizeof(MatchInfo));
  ASSERT_EQ(po->infoKey, i64);
  ASSERT_EQ(po->port , i16);
  ASSERT_EQ(po->attr , info.attr);
  buf.MakeNextPkg();
  ASSERT_TRUE(buf.IsEmptyPkgBody());
  ASSERT_EQ(buf.GetPkgBodyLen(), 0);
  ASSERT_EQ(buf.GetPkgLen(), buf.Getpkg_head_tLen());
  //ASSERT_EQ(buf.GetSendableLen(), pkgLen);

  //test GetSendableLen
  buf.Reset();
  uint32_t toto = 0;
  for (int i=0; i < 3; ++i)
{
    buf.AppendBuffer("12345", 6); //6
    ASSERT_EQ(buf.GetPkgBodyLen(), 6);
    ASSERT_EQ(buf.GetPkgLen(), 6 + buf.Getpkg_head_tLen());
    toto += buf.GetPkgLen();
    buf.MakeNextPkg();
    ASSERT_EQ(toto, buf.GetSendableLen());
}

 //test DiscardSendedData();
 ASSERT_TRUE(buf.IsEmptyPkgBody());
 ASSERT_TRUE(buf.m_pkgPos > 10);
 buf.m_readPos += 10; //send 10B
 buf.DiscardSendedData();
 ASSERT_TRUE(buf.IsEmptyPkgBody());


 buf.Reset();
 //uint32_t //有未发送数据，并且有未打包数据
 buf.AppendBuffer("123456", 7);
 buf.AppendBuffer("123456", 7);
 buf.AppendBuffer("123456", 7);
 buf.AppendBuffer("123456", 7);
 ASSERT_EQ(0, buf.GetSendableLen());
 buf.MakeNextPkg();
 ASSERT_NE(0, buf.GetSendableLen());

 //test 最后不足一个head的情况
{
    struct MsgHead {
        uint32_t a;
        uint32_t b;
        uint64_t c; //16
    };

  NetPkgSendBuf<MsgHead,_1K> sbuf;
  ASSERT_EQ(sizeof(MsgHead), 16);
  ASSERT_TRUE(sbuf.IsHasPkg());
  ASSERT_TRUE(sbuf.IsEmptyPkgBody());
  ASSERT_FALSE(sbuf.MakeNextPkg());
  ASSERT_TRUE(sbuf.IsEmptyPkgBody());
  ASSERT_EQ(sbuf.m_writePos, sizeof(MsgHead));
  ASSERT_EQ(1000, sbuf.AppendBuffer(data1K, 1000));
  ASSERT_EQ(1000, sbuf.GetPkgBodyLen());
  ASSERT_EQ(1000+sizeof(MsgHead), sbuf.GetPkgLen());
  ASSERT_TRUE(sbuf.IsHasPkg());
  ASSERT_EQ(8, sbuf.get_writable_len());
  //现在已写入1016字节，剩余8字节不足MsgHead
  ASSERT_FALSE(sbuf.MakeNextPkg());
  ASSERT_EQ(0, sbuf.get_writable_len());
  ASSERT_FALSE(sbuf.IsHasPkg());
  ASSERT_FALSE(sbuf.IsEmptyPkgBody());

  ASSERT_EQ(sbuf.GetSendableLen(), 1000+sizeof(MsgHead));
  //send
  sbuf.m_readPos = sbuf.m_pkgPos;
  ASSERT_EQ(sbuf.GetSendableLen(), 0);
  ASSERT_EQ(sbuf.DiscardSendedData(), 1000+sizeof(MsgHead));
  ASSERT_TRUE(sbuf.IsEmptyPkgBody());
  ASSERT_EQ(sbuf.m_writePos, sizeof(MsgHead));
  ASSERT_EQ(sbuf.m_pkgPos, 0);
  ASSERT_EQ(sbuf.m_readPos, 0);
  ASSERT_EQ(_1K-sizeof(MsgHead), sbuf.get_writable_len());
  ASSERT_EQ(sbuf.GetPkgBodyLen(), 0);
  ASSERT_EQ(sbuf.GetSendableLen(), 0);


  //验证DiscardSendedData, 一直添加数据,直到加满
  //NetPkgSendBuf<MsgHead,_1K> sbuf;
  sbuf.Reset();
  ASSERT_EQ(1000,sbuf.AppendBuffer(data1K,1000));
  //1024 - 1000 -16 = 8
  ASSERT_FALSE(sbuf.MakeNextPkg());
  ASSERT_FALSE(sbuf.IsHasPkg());
  ASSERT_EQ(sbuf.GetSendableLen(),1000+sizeof(MsgHead));
  ASSERT_EQ(sbuf.DiscardSendedData(), 0);
  sbuf.m_readPos = 3;
  ASSERT_EQ(sbuf.DiscardSendedData(), 3);
  ASSERT_FALSE(sbuf.IsHasPkg()); //8+3 =11
  sbuf.m_readPos = 4;
  ASSERT_EQ(sbuf.DiscardSendedData(), 4);
  ASSERT_FALSE(sbuf.IsHasPkg());
  sbuf.m_readPos=sbuf.m_pkgPos;
  ASSERT_EQ(sbuf.DiscardSendedData(), 1000+sizeof(MsgHead)-7);
  ASSERT_TRUE(sbuf.IsHasPkg());
  //ASSERT_EQ(sbuf.GetSendableLen(),1000+sizeof(MsgHead) - 8);
  //ASSERT_FALSE(sbuf.MakeNextPkg());
  printf("writePos:%u:%u:%u", sbuf.m_writePos, sbuf.m_pkgPos, sbuf.m_readPos);

}

 //TODO

 //ASSERT_TRUE(buf.DiscardPkgData() == 0);
 //ASSERT_TRUE(buf.DiscardPkgData() == 28);

}

#endif

