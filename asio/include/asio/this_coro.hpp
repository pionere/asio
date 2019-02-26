//
// this_coro.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_THIS_CORO_HPP
#define ASIO_THIS_CORO_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace this_coro {

/// Awaitable type that returns the executor of the current coroutine.
struct executor_t
{
  constexpr executor_t()
  {
  }
};

/// Awaitable object that returns the executor of the current coroutine.
constexpr executor_t executor;

} // namespace this_coro
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_THIS_CORO_HPP
