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

extern thread_local int  error;
extern thread_local char error_msg[1024];

#define esock_set_error_msg(fmt, ...) \
   snprintf(detail::error_msg, 1024, "%s:%d [%s] " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);
#define esock_set_syserr_msg(fmt, ...) \
   snprintf(detail::error_msg, 1024, "%s:%d [%s] (syserr:%s) " fmt , __FILE__, __LINE__, __func__, strerror(errno), ##__VA_ARGS__)

}

inline int get_errno() {return detail::error;}
inline const char* get_error_msg() {return detail::error_msg;}


}
