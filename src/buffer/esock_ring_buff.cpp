
/**
 * @brief Realy Ring Buffer.
 */

#include "../detail/esock_error.hpp"
#include "../detail/esock_sysinclude_fs.hpp"
#include "../detail/esock_utility.hpp"
#include "esock_ring_buff.hpp"

namespace esock
{


static inline long get_sys_pagesize ()
{
    long page_size = sysconf (_SC_PAGESIZE);
    if (page_size == -1)
        throw std::system_error (errno, std::system_category (), "sysconf(_SC_PAGESIZE) failed");

    return page_size;
}


/**
 * @brief alloc buff-size of physical memory then map to serial virtual address
 * twice.
 *        make base[n] == base[buff_size + n]
 * +-------------+-------------+
 * | A B C D E F | A B C D E F |
 * +-------------+-------------+
 *  \            x            /
 *    \        /   \        /
 *      \    /       \    /
 *       \ /          \ /
 *       +-------------+
 *       | A B C D E F |
 *       +-------------+
 *         buff_size
 *
 *
 * @param buff_size
 * @return NULL for alloc failed
 */
void *alloc_ring_buff (size_t buff_size)
{
    buff_size = align_up (buff_size, get_sys_pagesize ());

    pid_t tid = syscall (SYS_gettid);

    char shm_name[64];
    snprintf (shm_name, 64, "/esock_rring_alloc.%d", tid);

    int fd = shm_open (shm_name, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd == -1)
    {
        esock_report_syserr_msg ("shm_open failed");
        return nullptr;
    }
    shm_unlink (shm_name);

    // fd is still ok
    if (-1 == ftruncate (fd, buff_size))
    {
        esock_report_error_msg ("ftruncate failed");
        close (fd);
        return nullptr;
    }

    // get a two buff_size virtual space
    char *base =
    (char *)mmap (NULL, buff_size * 2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (MAP_FAILED == base)
    {
        esock_report_error_msg ("mmap failed");
        close (fd);
        return nullptr;
    }
    munmap (base, buff_size * 2);

    // remmap buff_size to two virtual space
    int i = 0;
    for (; i < 2; ++i)
    {
        if (MAP_FAILED == mmap (base + i * buff_size, buff_size, PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_FIXED, fd, 0))
        {
            esock_report_error_msg ("remmap buff idx:%d failed", i);
            if (i == 1)
            {
                munmap (base, buff_size);
                close (fd);
            }
            return nullptr;
        }
    }

    close (fd);
    return base;
}

void free_ring_buff (void *buff, size_t buff_size)
{
    // TODO assert align page_size
    buff_size = align_up(buff_size, get_sys_pagesize ());

    esock_assert_on (0 == munmap ((char *)buff, buff_size), DEBUG);
    esock_assert_on (0 == munmap ((char *)buff + buff_size, buff_size), DEBUG);
}

template <typename pkg_head_t, size_t BUFSIZE, size_t PKG_HEAD_LEN>
//net_recv_ringbuf_t<pkg_head_t, BUFSIZE, PKG_HEAD_LEN>::net_recv_ringbuf_t() throw
net_recv_ringbuf_t<pkg_head_t, BUFSIZE, PKG_HEAD_LEN>::net_recv_ringbuf_t()
{
    _bufsize = align_up (BUFSIZE, get_sys_pagesize());
    _buf = alloc_ring_buff(_bufsize);
    if (_buf == nullptr)
      throw std::bad_alloc("alloc_ring failed");
}

template <typename pkg_head_t, size_t BUFSIZE, size_t PKG_HEAD_LEN>
net_recv_ringbuf_t<pkg_head_t, BUFSIZE, PKG_HEAD_LEN>::~net_recv_ringbuf_t()
{
  if (_buf)
  {
    free_ring_buff(_buf, _bufsize);
    _buf = nullptr;
  }
  reset();
}


template <typename pkg_head_t, size_t BUFSIZE, size_t PKG_HEAD_LEN>
net_send_ringbuf_t<pkg_head_t, BUFSIZE, PKG_HEAD_LEN>::net_send_ringbuf_t()
{
    _bufsize = align_up (BUFSIZE, get_sys_pagesize());
    _buf = alloc_ring_buff(_bufsize);
    if (_buf == nullptr)
      throw std::bad_alloc("alloc_ring failed");
}

template <typename pkg_head_t, size_t BUFSIZE, size_t PKG_HEAD_LEN>
net_send_ringbuf_t<pkg_head_t, BUFSIZE, PKG_HEAD_LEN>::~net_send_ringbuf_t()
{
  if (_buf)
  {
    free_ring_buff(_buf, _bufsize);
    _buf = nullptr;
  }
  reset();

}


#ifdef ESOCK_TEST_ENV

//TODO



#endif

} // namespace nameesock
