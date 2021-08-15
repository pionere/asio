//
// detail/thread_info_base.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_THREAD_INFO_BASE_HPP
#define ASIO_DETAIL_THREAD_INFO_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <climits>
#include <cstddef>
#include "asio/detail/memory.hpp"
#include "asio/detail/noncopyable.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

#ifndef ASIO_RECYCLING_ALLOCATOR_CACHE_SIZE
# define ASIO_RECYCLING_ALLOCATOR_CACHE_SIZE 2
#endif // ASIO_RECYCLING_ALLOCATOR_CACHE_SIZE

class thread_info_base
  : private noncopyable
{
public:
  struct default_tag
  {
    enum
    {
      cache_size = ASIO_RECYCLING_ALLOCATOR_CACHE_SIZE,
      begin_mem_index = 0,
      end_mem_index = cache_size
    };
  };

  struct awaitable_frame_tag
  {
    enum
    {
      cache_size = ASIO_RECYCLING_ALLOCATOR_CACHE_SIZE,
      begin_mem_index = default_tag::end_mem_index,
      end_mem_index = begin_mem_index + cache_size
    };
  };

  struct executor_function_tag
  {
    enum
    {
      cache_size = ASIO_RECYCLING_ALLOCATOR_CACHE_SIZE,
      begin_mem_index = awaitable_frame_tag::end_mem_index,
      end_mem_index = begin_mem_index + cache_size
    };
  };

  enum { max_mem_index = executor_function_tag::end_mem_index };

  thread_info_base()
  {
    for (int i = 0; i < max_mem_index; ++i)
      reusable_memory_[i] = 0;
  }

  ~thread_info_base()
  {
    for (int i = 0; i < max_mem_index; ++i)
    {
      aligned_delete(reusable_memory_[i]);
    }
  }

  static void* allocate(thread_info_base* this_thread,
      std::size_t size, std::size_t align = ASIO_DEFAULT_ALIGN)
  {
    return allocate(default_tag(), this_thread, size, align);
  }

  static void deallocate(thread_info_base* this_thread,
      void* pointer, std::size_t size)
  {
    deallocate(default_tag(), this_thread, pointer, size);
  }

  template <typename Purpose>
  static void* allocate(Purpose, thread_info_base* this_thread,
      std::size_t size, std::size_t align = ASIO_DEFAULT_ALIGN)
  {
    std::size_t chunks = (size + chunk_size - 1) / chunk_size;

    if (this_thread)
    {
      for (int mem_index = Purpose::begin_mem_index;
          mem_index < Purpose::end_mem_index; ++mem_index)
      {
        if (this_thread->reusable_memory_[mem_index])
        {
          void* const pointer = this_thread->reusable_memory_[mem_index];
          unsigned char* const mem = static_cast<unsigned char*>(pointer);
          if (static_cast<std::size_t>(mem[0]) >= chunks
              && reinterpret_cast<std::size_t>(pointer) % align == 0)
          {
            this_thread->reusable_memory_[mem_index] = 0;
            mem[size] = mem[0];
            return pointer;
          }
        }
      }

      for (int mem_index = Purpose::begin_mem_index;
          mem_index < Purpose::end_mem_index; ++mem_index)
      {
        if (this_thread->reusable_memory_[mem_index])
        {
          void* const pointer = this_thread->reusable_memory_[mem_index];
          this_thread->reusable_memory_[mem_index] = 0;
          aligned_delete(pointer);
          break;
        }
      }
    }

    void* const pointer = aligned_new(align, chunks * chunk_size + 1);
    unsigned char* const mem = static_cast<unsigned char*>(pointer);
    mem[size] = (chunks <= UCHAR_MAX) ? static_cast<unsigned char>(chunks) : 0;
    return pointer;
  }

  template <typename Purpose>
  static void deallocate(Purpose, thread_info_base* this_thread,
      void* pointer, std::size_t size)
  {
    if (size <= chunk_size * UCHAR_MAX)
    {
      if (this_thread)
      {
        for (int mem_index = Purpose::begin_mem_index;
            mem_index < Purpose::end_mem_index; ++mem_index)
        {
          if (this_thread->reusable_memory_[mem_index] == 0)
          {
            unsigned char* const mem = static_cast<unsigned char*>(pointer);
            mem[0] = mem[size];
            this_thread->reusable_memory_[mem_index] = pointer;
            return;
          }
        }
      }
    }

    aligned_delete(pointer);
  }

private:
#if defined(ASIO_HAS_IO_URING)
  enum { chunk_size = 8 };
#else // defined(ASIO_HAS_IO_URING)
  enum { chunk_size = 4 };
#endif // defined(ASIO_HAS_IO_URING)
  void* reusable_memory_[max_mem_index];
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_THREAD_INFO_BASE_HPP
