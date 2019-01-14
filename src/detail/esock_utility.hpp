/**
 * @file     esock_utility.hpp
 *           
 *
 * @author   lili <lilijreey@gmail.com>
 * @date     01/05/2019 01:32:36 PM
 *
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <errno.h>
    
namespace esock { namespace detail {

#define esock_debug_log(fmt, ...) printf("%s:%d " fmt , __FILE__, __LINE__, ##__VA_ARGS__)


#define esock_assert(exp) assert(exp)

class noncopyable_t {

protected:
	noncopyable_t() {}
	~noncopyable_t() {}
private:
	noncopyable_t( const noncopyable_t& );
	const noncopyable_t& operator = ( const noncopyable_t& );
};

}} //nemespace
