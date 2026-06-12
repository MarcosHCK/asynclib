/* Copyright (C) 2025-2026 MarcosHCK
 * This file is part of asynclib.
 *
 * asynclib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * asynclib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once
#include <asynclib/future.h>
#include <asynclib/future_op.h>
#include <coroutine>
#include <type_traits>

namespace asynclib
{

  namespace details
    {

      template<Future _Future>
      struct __future_awaitable_base;
    }

  template<details::Future _Future> struct details::__future_awaitable_base
    {

      typedef _Future future_type;
      typedef unwrap_future_t<_Future> value_type;

      bool await_ready () const noexcept 
        { return _timeout != _future.wait_for (_zero); }

      value_type await_resume ()
        { return std::move (_future).get (); }

      void await_suspend (std::coroutine_handle<> handle)
        { _future >> [handle = std::move (handle)] (_Future&) noexcept { handle.resume (); }; }

      __future_awaitable_base (const __future_awaitable_base&) = delete;

      inline __future_awaitable_base (_Future&& future) noexcept: _future (std::move (future))
        { }

    private:

      future_type _future;

      static_assert(std::is_base_of_v<std::__future_base, _Future>);
      static constexpr auto _timeout = std::future_status::timeout;
      static constexpr auto _zero = std::chrono::milliseconds::zero ();
    };
}

namespace std
{

  template<typename T, typename... Args> struct coroutine_traits<std::future<T>, Args ...>
    {
      struct promise_type;
    };

  template<typename T, typename... Args> struct coroutine_traits<std::future<T>, Args ...>::promise_type
    {

      std::suspend_always final_suspend () noexcept
        { return { }; }

      std::future<T> get_return_object ()
        { return _promise.get_future (); }

      std::suspend_never initial_suspend ()
        { return { }; }
      
      void return_value (T value)
        { _promise.set_value (std::move (value)); }
      
      void unhandled_exception ()
        { _promise.set_exception (std::current_exception ()); }

      std::suspend_always await_transform (std::suspend_always)
        { return { }; }

      template<asynclib::details::Future _Future>
      static inline asynclib::details::__future_awaitable_base<_Future> await_transform (_Future&& future)
        {
          return asynclib::details::__future_awaitable_base<_Future> (std::forward<_Future> (future));
        }

      template<asynclib::details::Future _Future>
      static inline asynclib::details::__future_awaitable_base<std::shared_future<asynclib::unwrap_future_t<_Future>>> await_transform (_Future& future)
        {
          return asynclib::details::__future_awaitable_base<std::shared_future<asynclib::unwrap_future_t<_Future>>> (future.share ());
        }

    private:
      std::promise<T> _promise;
    };
}