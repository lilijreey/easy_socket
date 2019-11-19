/**
 * @file esock_sysinclude_fs.hpp
 * @author your name (you@domain.com)
 * @brief For accelerate compile
 * @date 2019-07-27
 *
 */

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <strings.h>

#include <chrono>
#include <new>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>
