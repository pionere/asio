
// impl/redirect_error.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IMPL_REDIRECT_ERROR_HPP
#define ASIO_IMPL_REDIRECT_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/associator.hpp"
#include "asio/async_result.hpp"
#include "asio/detail/handler_alloc_helpers.hpp"
#include "asio/detail/handler_cont_helpers.hpp"
#include "asio/detail/handler_invoke_helpers.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/system_error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

// Class to adapt a redirect_error_t as a completion handler.
template <typename Handler>
class redirect_error_handler
{
public:
  typedef void result_type;

  template <typename CompletionToken>
  redirect_error_handler(redirect_error_t<CompletionToken> e)
    : ec_(e.ec_),
      handler_(static_cast<CompletionToken&&>(e.token_))
  {
  }

  template <typename RedirectedHandler>
  redirect_error_handler(asio::error_code& ec,
      RedirectedHandler&& h)
    : ec_(ec),
      handler_(static_cast<RedirectedHandler&&>(h))
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
    handler_(
        static_cast<Arg&&>(arg),
        static_cast<Args&&>(args)...);
  }

  template <typename... Args>
  void operator()(const asio::error_code& ec, Args&&... args)
  {
    ec_ = ec;
    handler_(static_cast<Args&&>(args)...);
  }

//private:
  asio::error_code& ec_;
  Handler handler_;
};

template <typename Handler>
inline asio_handler_allocate_is_deprecated
asio_handler_allocate(std::size_t size,
    redirect_error_handler<Handler>* this_handler)
{
#if defined(ASIO_NO_DEPRECATED)
  asio_handler_alloc_helpers::allocate(size, this_handler->handler_);
  return asio_handler_allocate_is_no_longer_used();
#else // defined(ASIO_NO_DEPRECATED)
  return asio_handler_alloc_helpers::allocate(
      size, this_handler->handler_);
#endif // defined(ASIO_NO_DEPRECATED)
}

template <typename Handler>
inline asio_handler_deallocate_is_deprecated
asio_handler_deallocate(void* pointer, std::size_t size,
    redirect_error_handler<Handler>* this_handler)
{
  asio_handler_alloc_helpers::deallocate(
      pointer, size, this_handler->handler_);
#if defined(ASIO_NO_DEPRECATED)
  return asio_handler_deallocate_is_no_longer_used();
#endif // defined(ASIO_NO_DEPRECATED)
}

template <typename Handler>
inline bool asio_handler_is_continuation(
    redirect_error_handler<Handler>* this_handler)
{
  return asio_handler_cont_helpers::is_continuation(
        this_handler->handler_);
}

template <typename Function, typename Handler>
inline asio_handler_invoke_is_deprecated
asio_handler_invoke(Function& function,
    redirect_error_handler<Handler>* this_handler)
{
  asio_handler_invoke_helpers::invoke(
      function, this_handler->handler_);
#if defined(ASIO_NO_DEPRECATED)
  return asio_handler_invoke_is_no_longer_used();
#endif // defined(ASIO_NO_DEPRECATED)
}

template <typename Function, typename Handler>
inline asio_handler_invoke_is_deprecated
asio_handler_invoke(const Function& function,
    redirect_error_handler<Handler>* this_handler)
{
  asio_handler_invoke_helpers::invoke(
      function, this_handler->handler_);
#if defined(ASIO_NO_DEPRECATED)
  return asio_handler_invoke_is_no_longer_used();
#endif // defined(ASIO_NO_DEPRECATED)
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

# if defined(ASIO_HAS_REF_QUALIFIED_FUNCTIONS)

template <typename R, typename... Args>
struct redirect_error_signature<R(asio::error_code, Args...) &>
{
  typedef R type(Args...) &;
};

template <typename R, typename... Args>
struct redirect_error_signature<R(const asio::error_code&, Args...) &>
{
  typedef R type(Args...) &;
};

template <typename R, typename... Args>
struct redirect_error_signature<R(asio::error_code, Args...) &&>
{
  typedef R type(Args...) &&;
};

template <typename R, typename... Args>
struct redirect_error_signature<R(const asio::error_code&, Args...) &&>
{
  typedef R type(Args...) &&;
};

#  if defined(ASIO_HAS_NOEXCEPT_FUNCTION_TYPE)

template <typename R, typename... Args>
struct redirect_error_signature<
  R(asio::error_code, Args...) noexcept>
{
  typedef R type(Args...) & noexcept;
};

template <typename R, typename... Args>
struct redirect_error_signature<
  R(const asio::error_code&, Args...) noexcept>
{
  typedef R type(Args...) & noexcept;
};

template <typename R, typename... Args>
struct redirect_error_signature<
  R(asio::error_code, Args...) & noexcept>
{
  typedef R type(Args...) & noexcept;
};

template <typename R, typename... Args>
struct redirect_error_signature<
  R(const asio::error_code&, Args...) & noexcept>
{
  typedef R type(Args...) & noexcept;
};

template <typename R, typename... Args>
struct redirect_error_signature<
  R(asio::error_code, Args...) && noexcept>
{
  typedef R type(Args...) && noexcept;
};

template <typename R, typename... Args>
struct redirect_error_signature<
  R(const asio::error_code&, Args...) && noexcept>
{
  typedef R type(Args...) && noexcept;
};

#  endif // defined(ASIO_HAS_NOEXCEPT_FUNCTION_TYPE)
# endif // defined(ASIO_HAS_REF_QUALIFIED_FUNCTIONS)

} // namespace detail

#if !defined(GENERATING_DOCUMENTATION)

template <typename CompletionToken, typename Signature>
struct async_result<redirect_error_t<CompletionToken>, Signature>
{
  typedef typename async_result<CompletionToken,
    typename detail::redirect_error_signature<Signature>::type>
      ::return_type return_type;

  template <typename Initiation>
  struct init_wrapper
  {
    template <typename Init>
    init_wrapper(asio::error_code& ec, Init&& init)
      : ec_(ec),
        initiation_(static_cast<Init&&>(init))
    {
    }

    template <typename Handler, typename... Args>
    void operator()(Handler&& handler,
        Args&&... args)
    {
      static_cast<Initiation&&>(initiation_)(
          detail::redirect_error_handler<
            typename decay<Handler>::type>(
              ec_, static_cast<Handler&&>(handler)),
          static_cast<Args&&>(args)...);
    }

    asio::error_code& ec_;
    Initiation initiation_;
  };

  template <typename Initiation, typename RawCompletionToken, typename... Args>
  static return_type initiate(Initiation&& initiation,
      RawCompletionToken&& token, Args&&... args)
  {
    return async_initiate<CompletionToken,
      typename detail::redirect_error_signature<Signature>::type>(
        init_wrapper<typename decay<Initiation>::type>(
          token.ec_, static_cast<Initiation&&>(initiation)),
        token.token_, static_cast<Args&&>(args)...);
  }
};

template <template <typename, typename> class Associator,
    typename Handler, typename DefaultCandidate>
struct associator<Associator,
    detail::redirect_error_handler<Handler>, DefaultCandidate>
  : Associator<Handler, DefaultCandidate>
{
  static typename Associator<Handler, DefaultCandidate>::type get(
      const detail::redirect_error_handler<Handler>& h,
      const DefaultCandidate& c = DefaultCandidate()) noexcept
  {
    return Associator<Handler, DefaultCandidate>::get(h.handler_, c);
  }
};

#endif // !defined(GENERATING_DOCUMENTATION)

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_REDIRECT_ERROR_HPP
