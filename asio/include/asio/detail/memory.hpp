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
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <new>
#include "asio/detail/throw_exception.hpp"

#if !defined(ASIO_HAS_STD_ALIGNED_ALLOC) \
  && defined(ASIO_HAS_BOOST_ALIGN) \
  && defined(ASIO_HAS_ALIGNOF)
# include <boost/align/aligned_alloc.hpp>
#endif // !defined(ASIO_HAS_STD_ALIGNED_ALLOC)
       //   && defined(ASIO_HAS_BOOST_ALIGN)
       //   && defined(ASIO_HAS_ALIGNOF)

namespace asio {
namespace detail {

using std::shared_ptr;
using std::weak_ptr;
using std::addressof;

} // namespace detail

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

inline void* aligned_new(std::size_t align, std::size_t size)
{
#if defined(ASIO_HAS_STD_ALIGNED_ALLOC) && defined(ASIO_HAS_ALIGNOF)
  void* ptr = std::aligned_alloc(align, size);
  if (!ptr)
  {
    std::bad_alloc ex;
    asio::detail::throw_exception(ex);
  }
  return ptr;
#elif defined(ASIO_HAS_BOOST_ALIGN) && defined(ASIO_HAS_ALIGNOF)
  void* ptr = boost::alignment::aligned_alloc(align, size);
  if (!ptr)
  {
    std::bad_alloc ex;
    asio::detail::throw_exception(ex);
  }
  return ptr;
#elif defined(ASIO_HAS_MSVC) && defined(ASIO_HAS_ALIGNOF)
  void* ptr = _aligned_malloc(align, size);
  if (!ptr)
  {
    std::bad_alloc ex;
    asio::detail::throw_exception(ex);
  }
  return ptr;
#else // defined(ASIO_HAS_MSVC) && defined(ASIO_HAS_ALIGNOF)
  (void)align;
  return ::operator new(size);
#endif // defined(ASIO_HAS_MSVC) && defined(ASIO_HAS_ALIGNOF)
}

inline void aligned_delete(void* ptr)
{
#if defined(ASIO_HAS_STD_ALIGNED_ALLOC) && defined(ASIO_HAS_ALIGNOF)
  std::free(ptr);
#elif defined(ASIO_HAS_BOOST_ALIGN) && defined(ASIO_HAS_ALIGNOF)
  boost::alignment::aligned_free(ptr);
#elif defined(ASIO_HAS_MSVC) && defined(ASIO_HAS_ALIGNOF)
  _aligned_free(ptr);
#else // defined(ASIO_HAS_MSVC) && defined(ASIO_HAS_ALIGNOF)
  ::operator delete(ptr);
#endif // defined(ASIO_HAS_MSVC) && defined(ASIO_HAS_ALIGNOF)
}

} // namespace asio

#endif // ASIO_DETAIL_MEMORY_HPP
