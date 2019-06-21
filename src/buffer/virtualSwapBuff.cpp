/*
 * 一种环行队列, 支持不短往后写功能，
 * 能够一直保持数据局部连续.
 *
 */


#define _BSD_SOURCE  

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
       #include <string.h>
       #include <assert.h>
       #include <unistd.h>
       #include <fcntl.h>	    /* For O_* constants */
       #include <sys/mman.h>
       #include <sys/stat.h>	    /* For mode constants */
       #include <sys/types.h>

/*
 * TripleKill Buffer
 *
 * 

 一般环行缓存写完后有两种方式
 1. 回绕的头写
 +-+-+-+-+-+-+-+-+
 | | | | | |4|5|6|
 +-+-+-+-+-+-+-+-+
 +-+-+-+-+-+-+-+-+
 |7|8| | | |4|5|6| 从头写入
 +-+-+-+-+-+-+-+-+

 2. 前移未消费数据腾出尾部空间后写入
 +-+-+-+-+-+-+-+-+
 | | | | | |4|5|6|
 +-+-+-+-+-+-+-+-+
 +-+-+-+-+-+-+-+-+
 |4|5|6| | | | | |  移动
 +-+-+-+-+-+-+-+-+
 +-+-+-+-+-+-+-+-+
 |4|5|6|7|8| | | |  写入
 +-+-+-+-+-+-+-+-+

 对于接受网络数据用来解析来说，上面两种都存在一些缺点
 1. 第一种写回绕了，对于上层包解析来说一般都需要地址连续的的数据。
    但是我们在接受数据时一般并不知道应用层数据信息，所以无法在保证一条应用层数据不被回绕

  2. 为了解决回绕问题一般采用第二种方式，当尾部写满后，平移还未消费数据到buffer头以腾出有效空间。
     这种方式虽然解决了数据回绕问题但显然会有移动数据的消耗。 当然我们可以做一些优化尽量减少数据拷贝。
    但是不管怎么优化都是在数据拷贝量和可用空间中做权衡。 无法做在不回绕的情况下到既能使用全部空间有没有数据拷贝.

  是否存在一个工程上可行的方案使我们鱼和熊掌兼得。
  经过一些思索后


 环行buff内存布局
+--------------+--------------+--------------+
|     m1       |     m2       |    m1v       |
+--------------+--------------+--------------+
 m1 m2是两块大小相同长度是页倍数的buffer, 
 m1v 与m1映射的是同一段物理内存
 

*/

struct tkbuff
{
    //必须是pageSize的偶数倍
    static tkbuff* make(size_t buff_size);

 private:
  tkbuff();
  tkbuff(tkbuff &);
  ~tkbuff()
  {
      //TODO

  }

    size_t _buff_sz;
    size_t _block_sz;
    uint8_t *_buff;

};



tkbuff* tkbuff::make(size_t buff_size)
{
    if (buff_size % PAGE_SIZE)
    {
        //errno = INVALID;
        return NULL;
    }
    // TODO check buff_size

}

template <class T>
T* addr_add(T *addr, int bytes)
{
    return (T*)((uintptr_t)addr + bytes);
}

template <class T>
void addr_forward(T *&addr, int bytes)
{
    uintptr_t i = (uintptr_t)addr + bytes;;
    addr = i;
}

void *alloc_buff(size_t buff_size)
{
    const int blocksz = buff_size /2;

    void *block1= 0;
    void *block2= 0;
    void *block1v= 0;

    pid_t tid = gettid();
    char shm_name[64];
    snprintf(shm_name, 64, "/esock_tkbuff.%d", tid);

    int fd = shm_open(shm_name, O_RDWR|O_CREAT|O_TRUNC, 0600);
    if (fd == -1) {
        return NULL;
    }

    shm_unlink(buff_size);

    //fd is still ok
    if (-1 == ftruncate(fd, buff_size))
        goto closefd;


    //get a will mapping memory space
    block1 = mmap(NULL, blocksz*3, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS , -1, 0);
    if ((void*)-1 == base)
        goto closefd;

    munmap(block1, blocksz*3);

    //remmap
    block1 = mmap(block1, blocksz, PROT_READ | PROT_WRITE, MAP_SHARED |MAP_FIXED, fd, 0);
    if ((void*)-1 == block1) 
        goto closefd;

    block2 = addr_add(block1, blocksz);
    block2= mmap(block2, blocksz, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_FIXED, -1, 0);
    if ((void*)-1 == block2)
        goto failed;

    block1v = addr_add(block2, blocksz);
    block1v = mmap(block1v, blocksz, PROT_READ | PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, 0);
    if ((void*)-1 == block1v)
        goto failed;


    close(fd);

    return block1;

failed:
    //TODO save errno
    close(fd);
    if (block1v) munmap(block1v, blocksz);
    if (block2) munmap(block2, blocksz);
    if (block1) munmap(block1, blocksz);
    return NULL;
}

void free_buff(void *buff, size_t blocksz)
{
    intptr_t addr = (intptr_t)buff;
    for (int i=0; i<3; ++i)
    {
        munmap(addr, blocksz);
        addr += blocksz;
    }

}

int main()
{

    printf("base1:%p\n",base);
    printf("base2:%p\n",base2);

    memcpy(base, "hello", 6);
    assert(memcmp(base,base2, 6) == 0);

    memcpy(base2, "world", 6);
    assert(memcmp(base,base2, 6) == 0);

    /*GenRingBuffer*/
    
    
    return 0;
}
