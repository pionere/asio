//
// experimental/impl/redirect_error.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXPERIMENTAL_IMPL_REDIRECT_ERROR_HPP
#define ASIO_EXPERIMENTAL_IMPL_REDIRECT_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/associated_executor.hpp"
#include "asio/associated_allocator.hpp"
#include "asio/async_result.hpp"
#include "asio/detail/handler_alloc_helpers.hpp"
#include "asio/detail/handler_cont_helpers.hpp"
#include "asio/detail/handler_invoke_helpers.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/handler_type.hpp"
#include "asio/system_error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace experimental {
namespace detail {

// Class to adapt a redirect_error_t as a completion handler.
template <typename Handler>
class redirect_error_handler
{
public:
  template <typename CompletionToken>
  redirect_error_handler(redirect_error_t<CompletionToken> e)
    : ec_(e.ec_),
      handler_(static_cast<CompletionToken&&>(e.token_))
  {
  }

  void operator()()
  {
    handler_();
  }

  template <typename Arg, typename... Args>
  typename enable_if<
    !is_same<typename decay<Arg>::type, asio::error_code>::value
  >::type
  operator()(Arg&& arg, Args&&... args)
  {
    handler_(static_cast<Arg&&>(arg),
        static_cast<Args&&>(args)...);
  }

  template <typename... Args>
  void operator()(const asio::error_code& ec,
      Args&&... args)
  {
    ec_ = ec;
    handler_(static_cast<Args&&>(args)...);
  }

//private:
  asio::error_code& ec_;
  Handler handler_;
};

template <typename Handler>
inline void* asio_handler_allocate(std::size_t size,
    redirect_error_handler<Handler>* this_handler)
{
  return asio_handler_alloc_helpers::allocate(
      size, this_handler->handler_);
}

template <typename Handler>
inline void asio_handler_deallocate(void* pointer, std::size_t size,
    redirect_error_handler<Handler>* this_handler)
{
  asio_handler_alloc_helpers::deallocate(
      pointer, size, this_handler->handler_);
}

template <typename Handler>
inline bool asio_handler_is_continuation(
    redirect_error_handler<Handler>* this_handler)
{
  return asio_handler_cont_helpers::is_continuation(
        this_handler->handler_);
}

template <typename Function, typename Handler>
inline void asio_handler_invoke(Function& function,
    redirect_error_handler<Handler>* this_handler)
{
  asio_handler_invoke_helpers::invoke(
      function, this_handler->handler_);
}

template <typename Function, typename Handler>
inline void asio_handler_invoke(const Function& function,
    redirect_error_handler<Handler>* this_handler)
{
  asio_handler_invoke_helpers::invoke(
      function, this_handler->handler_);
}

template <typename Signature>
struct redirect_error_signature
{
  typedef Signature type;
};

template <typename R, typename... Args>
struct redirect_error_signature<R(asio::error_code, Args...)>
{
  typedef R type(Args...);
};

template <typename R, typename... Args>
struct redirect_error_signature<R(const asio::error_code&, Args...)>
{
  typedef R type(Args...);
};

} // namespace detail
} // namespace experimental

#if !defined(GENERATING_DOCUMENTATION)

template <typename CompletionToken, typename Signature>
struct async_result<experimental::redirect_error_t<CompletionToken>, Signature>
  : async_result<CompletionToken,
      typename experimental::detail::redirect_error_signature<Signature>::type>
{
  typedef experimental::detail::redirect_error_handler<
    typename async_result<CompletionToken,
      typename experimental::detail::redirect_error_signature<Signature>::type>
        ::completion_handler_type> completion_handler_type;

  explicit async_result(completion_handler_type& h)
    : async_result<CompletionToken,
        typename experimental::detail::redirect_error_signature<
          Signature>::type>(h.handler_)
  {
  }
};

#if !defined(ASIO_NO_DEPRECATED)

template <typename CompletionToken, typename Signature>
struct handler_type<experimental::redirect_error_t<CompletionToken>, Signature>
{
  typedef experimental::detail::redirect_error_handler<
    typename async_result<CompletionToken,
      typename experimental::detail::redirect_error_signature<Signature>::type>
        ::completion_handler_type> type;
};

template <typename Handler>
struct async_result<experimental::detail::redirect_error_handler<Handler> >
  : async_result<Handler>
{
  explicit async_result(
      experimental::detail::redirect_error_handler<Handler>& h)
    : async_result<Handler>(h.handler_)
  {
  }
};

#endif // !defined(ASIO_NO_DEPRECATED)

template <typename Handler, typename Executor>
struct associated_executor<
    experimental::detail::redirect_error_handler<Handler>, Executor>
{
  typedef typename associated_executor<Handler, Executor>::type type;

  static type get(
      const experimental::detail::redirect_error_handler<Handler>& h,
      const Executor& ex = Executor()) noexcept
  {
    return associated_executor<Handler, Executor>::get(h.handler_, ex);
  }
};

template <typename Handler, typename Allocator>
struct associated_allocator<
    experimental::detail::redirect_error_handler<Handler>, Allocator>
{
  typedef typename associated_allocator<Handler, Allocator>::type type;

  static type get(
      const experimental::detail::redirect_error_handler<Handler>& h,
      const Allocator& a = Allocator()) noexcept
  {
    return associated_allocator<Handler, Allocator>::get(h.handler_, a);
  }
};

#endif // !defined(GENERATING_DOCUMENTATION)

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXPERIMENTAL_IMPL_REDIRECT_ERROR_HPP
