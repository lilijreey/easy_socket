#include "esock_error.hpp"

namespace esock { 


namespace detail {

handle_esock_error_report_fn_t error_report_fn=0;

} //end namespace detail


handle_esock_error_report_fn_t set_error_report_fn(handle_esock_error_report_fn_t now)
{
    auto old = detail::error_report_fn;
    detail::error_report_fn = now;
    return old;
}


}
