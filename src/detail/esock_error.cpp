#include "esock_error.hpp"

namespace esock {

handle_esock_error_report_fn_t error_report_fn = 0;

handle_esock_error_report_fn_t set_error_report_fn (handle_esock_error_report_fn_t now)
{
    auto old = error_report_fn;
    error_report_fn = now;
    return old;
}

}
