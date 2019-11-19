/**
 * @file     esock_utility.hpp
 *
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/05/2019 01:32:36 PM
 *
 */

#pragma once

#include "esock_sysinclude_fs.hpp"

namespace esock
{

/* align addr on a size boundary - adjust address up/down if needed */
// TODO test
template <typename T> inline constexpr T align_up (T size, long align_size) noexcept
{
    static_assert (std::is_integral<T>::value, "T type error");
    return ((size + align_size) & ~align_size);
}

template <typename T> inline constexpr T align_down (T size, long align_size) noexcept
{
    static_assert (std::is_integral<T>::value, "T type error");
    return size & ~(T) (align_size - 1);
}

namespace detail
{

class noncopyable_t
{
    protected:
    noncopyable_t () {}
    ~noncopyable_t () {}

    private:
    noncopyable_t (const noncopyable_t &);
    const noncopyable_t &operator= (const noncopyable_t &);
};

} // namespace detail

} // nemespace
