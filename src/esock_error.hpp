/**
 * @file     esock_error.hpp
 *           
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/05/2019 08:10:12 PM
 *
 */

#pragma once
#include <string.h>
#include <stdio.h>

namespace esock { 

namespace detail {
#define esock_set_error_msg(fmt, ...) printf("%s:%d [%s] " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__);

#define esock_set_syserr_msg(fmt, ...) \
  printf("%s:%d [%s] (syserr:%s) " fmt "\n", __FILE__, __LINE__, __func__, strerror(errno), ##__VA_ARGS__)

#define esock_debug_log printf
//extern __local int  esock_errno;
//extern __local char esock_error_msg[1024];
}

//static inline const char* get_error_msg() {return detail::esock_error_msg;}
//static inline int get_errno() {return detail::esock_errno;}



}
