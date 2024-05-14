//
// detail/memory.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_MEMORY_HPP
#define ASIO_DETAIL_MEMORY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <memory>

namespace asio {
namespace detail {

using std::shared_ptr;
using std::weak_ptr;
using std::addressof;

} // namespace detail

#if defined(ASIO_HAS_CXX11_ALLOCATORS)
using std::allocator_arg_t;
# define ASIO_USES_ALLOCATOR(t) \
  namespace std { \
    template <typename Allocator> \
    struct uses_allocator<t, Allocator> : true_type {}; \
  } \
  /**/
# define ASIO_REBIND_ALLOC(alloc, t) \
  typename std::allocator_traits<alloc>::template rebind_alloc<t>
  /**/
#else // defined(ASIO_HAS_CXX11_ALLOCATORS)
struct allocator_arg_t {};
# define ASIO_USES_ALLOCATOR(t)
# define ASIO_REBIND_ALLOC(alloc, t) \
  typename alloc::template rebind<t>::other
  /**/
#endif // defined(ASIO_HAS_CXX11_ALLOCATORS)

} // namespace asio

#endif // ASIO_DETAIL_MEMORY_HPP
