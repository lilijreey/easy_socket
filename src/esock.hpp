/**
 * @file     esock.hpp
 *
 *
 */

#include "detail/esock_error.hpp"
#include "detail/esock_sockpool.hpp"
#include "esock_engine.hpp"
#include "esock_tcp_listener.hpp"
#include "esock_async_tcp_connector.hpp"
#include "esock_async_tcp_connection.hpp"
#include "esock_async_tcp_client.hpp"
#include "esock_async_tcp_server.hpp"
#include "esock_async_udp_client.hpp"

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
