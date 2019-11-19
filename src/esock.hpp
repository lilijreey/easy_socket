/**
 * @file     esock.hpp
 *
 *
 */

#include "buffer/esock_netbuff.hpp"
#include "detail/esock_error.hpp"
#include "detail/esock_sockpool.hpp"
#include "esock_engine.hpp"

namespace esock
{
// golabl functions

/**
 * @brief esock do not to log error msg, so you can set a function, when error
 * occurs fn will be called.
 * @param fn null or fn
 * @return old fn
 */
handle_esock_error_report_fn_t set_error_report_fn (handle_esock_error_report_fn_t fn);

// TODO when fd limit changed change sockpool size
}
