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
#include "esock_async_udp_handler.hpp"
#include "esock_async_unix_dgram_handler.hpp"

namespace esock
{

using handle_esock_error_report_fn_t = void (*) (const char *error_msg);
extern handle_esock_error_report_fn_t error_report_fn;
/**
 * @brief esock do not to log error msg, so you can set a function, when error
 * occurs fn will be called.
 * @param fn null or fn
 * @return old fn
 */
handle_esock_error_report_fn_t set_error_report_fn (handle_esock_error_report_fn_t fn);

}
