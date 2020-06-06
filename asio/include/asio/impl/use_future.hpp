//
// impl/use_future.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IMPL_USE_FUTURE_HPP
#define ASIO_IMPL_USE_FUTURE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <tuple>
#include "asio/async_result.hpp"
#include "asio/detail/memory.hpp"
#include "asio/error_code.hpp"
#include "asio/packaged_task.hpp"
#include "asio/system_error.hpp"
#include "asio/system_executor.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename T, typename F, typename... Args>
inline void promise_invoke_and_set(std::promise<T>& p,
    F& f, Args&&... args)
{
#if !defined(ASIO_NO_EXCEPTIONS)
  try
#endif // !defined(ASIO_NO_EXCEPTIONS)
  {
    p.set_value(f(static_cast<Args&&>(args)...));
  }
#if !defined(ASIO_NO_EXCEPTIONS)
  catch (...)
  {
    p.set_exception(std::current_exception());
  }
#endif // !defined(ASIO_NO_EXCEPTIONS)
}

template <typename F, typename... Args>
inline void promise_invoke_and_set(std::promise<void>& p,
    F& f, Args&&... args)
{
#if !defined(ASIO_NO_EXCEPTIONS)
  try
#endif // !defined(ASIO_NO_EXCEPTIONS)
  {
    f(static_cast<Args&&>(args)...);
    p.set_value();
  }
#if !defined(ASIO_NO_EXCEPTIONS)
  catch (...)
  {
    p.set_exception(std::current_exception());
  }
#endif // !defined(ASIO_NO_EXCEPTIONS)
}

// A function object adapter to invoke a nullary function object and capture
// any exception thrown into a promise.
template <typename T, typename F>
class promise_invoker
{
public:
  promise_invoker(const shared_ptr<std::promise<T>>& p,
      F&& f)
    : p_(p), f_(static_cast<F&&>(f))
  {
  }

  void operator()()
  {
#if !defined(ASIO_NO_EXCEPTIONS)
    try
#endif // !defined(ASIO_NO_EXCEPTIONS)
    {
      f_();
    }
#if !defined(ASIO_NO_EXCEPTIONS)
    catch (...)
    {
      p_->set_exception(std::current_exception());
    }
#endif // !defined(ASIO_NO_EXCEPTIONS)
  }

private:
  shared_ptr<std::promise<T> > p_;
  typename decay<F>::type f_;
};

// An executor that adapts the system_executor to capture any exeption thrown
// by a submitted function object and save it into a promise.
template <typename T>
class promise_executor
{
public:
  explicit promise_executor(const shared_ptr<std::promise<T>>& p)
    : p_(p)
  {
  }

  execution_context& context() const noexcept
  {
    return system_executor().context();
  }

  void on_work_started() const noexcept {}
  void on_work_finished() const noexcept {}

  template <typename F, typename A>
  void dispatch(F&& f, const A&) const
  {
    promise_invoker<T, F>(p_, static_cast<F&&>(f))();
  }

  template <typename F, typename A>
  void post(F&& f, const A& a) const
  {
    system_executor().post(
        promise_invoker<T, F>(p_, static_cast<F&&>(f)), a);
  }

  template <typename F, typename A>
  void defer(F&& f, const A& a) const
  {
    system_executor().defer(
        promise_invoker<T, F>(p_, static_cast<F&&>(f)), a);
  }

  friend bool operator==(const promise_executor& a,
      const promise_executor& b) noexcept
  {
    return a.p_ == b.p_;
  }

  friend bool operator!=(const promise_executor& a,
      const promise_executor& b) noexcept
  {
    return a.p_ != b.p_;
  }

private:
  shared_ptr<std::promise<T>> p_;
};

// The base class for all completion handlers that create promises.
template <typename T>
class promise_creator
{
public:
  typedef promise_executor<T> executor_type;

  executor_type get_executor() const noexcept
  {
    return executor_type(p_);
  }

  typedef std::future<T> future_type;

  future_type get_future()
  {
    return p_->get_future();
  }

protected:
  template <typename Allocator>
  void create_promise(const Allocator& a)
  {
    ASIO_REBIND_ALLOC(Allocator, char) b(a);
    p_ = std::allocate_shared<std::promise<T>>(b, std::allocator_arg, b);
  }

  shared_ptr<std::promise<T>> p_;
};

// For completion signature void().
class promise_handler_0
  : public promise_creator<void>
{
public:
  void operator()()
  {
    this->p_->set_value();
  }
};

// For completion signature void(error_code).
class promise_handler_ec_0
  : public promise_creator<void>
{
public:
  void operator()(const asio::error_code& ec)
  {
    if (ec)
    {
      this->p_->set_exception(
          std::make_exception_ptr(
            asio::system_error(ec)));
    }
    else
    {
      this->p_->set_value();
    }
  }
};

// For completion signature void(exception_ptr).
class promise_handler_ex_0
  : public promise_creator<void>
{
public:
  void operator()(const std::exception_ptr& ex)
  {
    if (ex)
    {
      this->p_->set_exception(ex);
    }
    else
    {
      this->p_->set_value();
    }
  }
};

// For completion signature void(T).
template <typename T>
class promise_handler_1
  : public promise_creator<T>
{
public:
  template <typename Arg>
  void operator()(Arg&& arg)
  {
    this->p_->set_value(static_cast<Arg&&>(arg));
  }
};

// For completion signature void(error_code, T).
template <typename T>
class promise_handler_ec_1
  : public promise_creator<T>
{
public:
  template <typename Arg>
  void operator()(const asio::error_code& ec,
      Arg&& arg)
  {
    if (ec)
    {
      this->p_->set_exception(
          std::make_exception_ptr(
            asio::system_error(ec)));
    }
    else
      this->p_->set_value(static_cast<Arg&&>(arg));
  }
};

// For completion signature void(exception_ptr, T).
template <typename T>
class promise_handler_ex_1
  : public promise_creator<T>
{
public:
  template <typename Arg>
  void operator()(const std::exception_ptr& ex,
      Arg&& arg)
  {
    if (ex)
      this->p_->set_exception(ex);
    else
      this->p_->set_value(static_cast<Arg&&>(arg));
  }
};

// For completion signature void(T1, ..., Tn);
template <typename T>
class promise_handler_n
  : public promise_creator<T>
{
public:
  template <typename... Args>
  void operator()(Args&&... args)
  {
    this->p_->set_value(
        std::forward_as_tuple(
          static_cast<Args&&>(args)...));
  }
};

// For completion signature void(error_code, T1, ..., Tn);
template <typename T>
class promise_handler_ec_n
  : public promise_creator<T>
{
public:
  template <typename... Args>
  void operator()(const asio::error_code& ec, Args&&... args)
  {
    if (ec)
    {
      this->p_->set_exception(
          std::make_exception_ptr(
            asio::system_error(ec)));
    }
    else
    {
      this->p_->set_value(
          std::forward_as_tuple(
            static_cast<Args&&>(args)...));
    }
  }
};

// For completion signature void(exception_ptr, T1, ..., Tn);
template <typename T>
class promise_handler_ex_n
  : public promise_creator<T>
{
public:
  template <typename... Args>
  void operator()(const std::exception_ptr& ex,
      Args&&... args)
  {
    if (ex)
      this->p_->set_exception(ex);
    else
    {
      this->p_->set_value(
          std::forward_as_tuple(
            static_cast<Args&&>(args)...));
    }
  }
};

// Helper template to choose the appropriate concrete promise handler
// implementation based on the supplied completion signature.
template <typename> class promise_handler_selector;

template <>
class promise_handler_selector<void()>
  : public promise_handler_0 {};

template <>
class promise_handler_selector<void(asio::error_code)>
  : public promise_handler_ec_0 {};

template <>
class promise_handler_selector<void(std::exception_ptr)>
  : public promise_handler_ex_0 {};

template <typename Arg>
class promise_handler_selector<void(Arg)>
  : public promise_handler_1<Arg> {};

template <typename Arg>
class promise_handler_selector<void(asio::error_code, Arg)>
  : public promise_handler_ec_1<Arg> {};

template <typename Arg>
class promise_handler_selector<void(std::exception_ptr, Arg)>
  : public promise_handler_ex_1<Arg> {};

template <typename... Arg>
class promise_handler_selector<void(Arg...)>
  : public promise_handler_n<std::tuple<Arg...>> {};

template <typename... Arg>
class promise_handler_selector<void(asio::error_code, Arg...)>
  : public promise_handler_ec_n<std::tuple<Arg...>> {};

template <typename... Arg>
class promise_handler_selector<void(std::exception_ptr, Arg...)>
  : public promise_handler_ex_n<std::tuple<Arg...>> {};

// Completion handlers produced from the use_future completion token, when not
// using use_future::operator().
template <typename Signature, typename Allocator>
class promise_handler
  : public promise_handler_selector<Signature>
{
public:
  typedef Allocator allocator_type;
  typedef void result_type;

  promise_handler(use_future_t<Allocator> u)
    : allocator_(u.get_allocator())
  {
    this->create_promise(allocator_);
  }

  allocator_type get_allocator() const noexcept
  {
    return allocator_;
  }

private:
  Allocator allocator_;
};

#if !defined(ASIO_NO_DEPRECATED)

template <typename Function, typename Signature, typename Allocator>
inline void asio_handler_invoke(Function& f,
    promise_handler<Signature, Allocator>* h)
{
  typename promise_handler<Signature, Allocator>::executor_type
    ex(h->get_executor());
  ex.dispatch(static_cast<Function&&>(f), std::allocator<void>());
}

template <typename Function, typename Signature, typename Allocator>
inline void asio_handler_invoke(const Function& f,
    promise_handler<Signature, Allocator>* h)
{
  typename promise_handler<Signature, Allocator>::executor_type
    ex(h->get_executor());
  ex.dispatch(f, std::allocator<void>());
}

#endif // !defined(ASIO_NO_DEPRECATED)

// Helper base class for async_result specialisation.
template <typename Signature, typename Allocator>
class promise_async_result
{
public:
  typedef promise_handler<Signature, Allocator> completion_handler_type;
  typedef typename completion_handler_type::future_type return_type;

  explicit promise_async_result(completion_handler_type& h)
    : future_(h.get_future())
  {
  }

  return_type get()
  {
    return static_cast<return_type&&>(future_);
  }

private:
  return_type future_;
};

// Return value from use_future::operator().
template <typename Function, typename Allocator>
class packaged_token
{
public:
  packaged_token(Function f, const Allocator& a)
    : function_(static_cast<Function&&>(f)),
      allocator_(a)
  {
  }

//private:
  Function function_;
  Allocator allocator_;
};

// Completion handlers produced from the use_future completion token, when
// using use_future::operator().
template <typename Function, typename Allocator, typename Result>
class packaged_handler
  : public promise_creator<Result>
{
public:
  typedef Allocator allocator_type;
  typedef void result_type;

  packaged_handler(packaged_token<Function, Allocator> t)
    : function_(static_cast<Function&&>(t.function_)),
      allocator_(t.allocator_)
  {
    this->create_promise(allocator_);
  }

  allocator_type get_allocator() const noexcept
  {
    return allocator_;
  }

  template <typename... Args>
  void operator()(Args&&... args)
  {
    (promise_invoke_and_set)(*this->p_,
        function_, static_cast<Args&&>(args)...);
  }

private:
  Function function_;
  Allocator allocator_;
};

#if !defined(ASIO_NO_DEPRECATED)

template <typename Function,
    typename Function1, typename Allocator, typename Result>
inline void asio_handler_invoke(Function& f,
    packaged_handler<Function1, Allocator, Result>* h)
{
  typename packaged_handler<Function1, Allocator, Result>::executor_type
    ex(h->get_executor());
  ex.dispatch(static_cast<Function&&>(f), std::allocator<void>());
}

template <typename Function,
    typename Function1, typename Allocator, typename Result>
inline void asio_handler_invoke(const Function& f,
    packaged_handler<Function1, Allocator, Result>* h)
{
  typename packaged_handler<Function1, Allocator, Result>::executor_type
    ex(h->get_executor());
  ex.dispatch(f, std::allocator<void>());
}

#endif // !defined(ASIO_NO_DEPRECATED)

// Helper base class for async_result specialisation.
template <typename Function, typename Allocator, typename Result>
class packaged_async_result
{
public:
  typedef packaged_handler<Function, Allocator, Result> completion_handler_type;
  typedef typename completion_handler_type::future_type return_type;

  explicit packaged_async_result(completion_handler_type& h)
    : future_(h.get_future())
  {
  }

  return_type get()
  {
    return static_cast<return_type&&>(future_);
  }

private:
  return_type future_;
};

} // namespace detail

template <typename Allocator> template <typename Function>
inline detail::packaged_token<typename decay<Function>::type, Allocator>
use_future_t<Allocator>::operator()(Function&& f) const
{
  return detail::packaged_token<typename decay<Function>::type, Allocator>(
      static_cast<Function&&>(f), allocator_);
}

#if !defined(GENERATING_DOCUMENTATION)

template <typename Allocator, typename Result, typename... Args>
class async_result<use_future_t<Allocator>, Result(Args...)>
  : public detail::promise_async_result<
      void(typename decay<Args>::type...), Allocator>
{
public:
  explicit async_result(
    typename detail::promise_async_result<void(typename decay<Args>::type...),
      Allocator>::completion_handler_type& h)
    : detail::promise_async_result<
        void(typename decay<Args>::type...), Allocator>(h)
  {
  }
};

template <typename Function, typename Allocator,
    typename Result, typename... Args>
class async_result<detail::packaged_token<Function, Allocator>, Result(Args...)>
  : public detail::packaged_async_result<Function, Allocator,
      typename result_of<Function(Args...)>::type>
{
public:
  explicit async_result(
    typename detail::packaged_async_result<Function, Allocator,
      typename result_of<Function(Args...)>::type>::completion_handler_type& h)
    : detail::packaged_async_result<Function, Allocator,
        typename result_of<Function(Args...)>::type>(h)
  {
  }
};

#if !defined(ASIO_NO_DEPRECATED)

template <typename Allocator, typename Signature>
struct handler_type<use_future_t<Allocator>, Signature>
{
  typedef typename async_result<use_future_t<Allocator>,
    Signature>::completion_handler_type type;
};

template <typename Signature, typename Allocator>
class async_result<detail::promise_handler<Signature, Allocator> >
  : public detail::promise_async_result<Signature, Allocator>
{
public:
  typedef typename detail::promise_async_result<
    Signature, Allocator>::return_type type;

  explicit async_result(
    typename detail::promise_async_result<
      Signature, Allocator>::completion_handler_type& h)
    : detail::promise_async_result<Signature, Allocator>(h)
  {
  }
};

template <typename Function, typename Allocator, typename Signature>
struct handler_type<detail::packaged_token<Function, Allocator>, Signature>
{
  typedef typename async_result<detail::packaged_token<Function, Allocator>,
    Signature>::completion_handler_type type;
};

template <typename Function, typename Allocator, typename Result>
class async_result<detail::packaged_handler<Function, Allocator, Result> >
  : public detail::packaged_async_result<Function, Allocator, Result>
{
public:
  typedef typename detail::packaged_async_result<
    Function, Allocator, Result>::return_type type;

  explicit async_result(
    typename detail::packaged_async_result<
      Function, Allocator, Result>::completion_handler_type& h)
    : detail::packaged_async_result<Function, Allocator, Result>(h)
  {
  }
};

#endif // !defined(ASIO_NO_DEPRECATED)

#endif // !defined(GENERATING_DOCUMENTATION)

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_USE_FUTURE_HPP
