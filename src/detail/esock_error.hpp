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

using handle_esock_error_report_fn_t = void (*)(const char* error_msg);


namespace detail {

extern handle_esock_error_report_fn_t error_report_fn;

}

} //end namespace eosck

#define esock_report_error_msg(fmt, ...) \
    do { \
        if (esock::detail::error_report_fn) { \
            char __esock_error_msg[256]; \
            snprintf(__esock_error_msg, 256, "%s:%d [%s] " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
            esock::detail::error_report_fn(__esock_error_msg); \
        } \
    }while(0)


#define esock_report_syserr_msg(fmt, ...) \
    do { \
        if (esock::detail::error_report_fn) { \
            char __esock_error_msg[256]; \
            snprintf(__esock_error_msg, 256, "%s:%d [%s] (syserr:%s) " fmt, __FILE__, __LINE__, __func__, strerror(errno), ##__VA_ARGS__); \
            esock::detail::error_report_fn(__esock_error_msg); \
        } \
    }while(0)
