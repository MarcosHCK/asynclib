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
#include <concepts>
#include <coroutine>
#include <type_traits>

namespace asynclib
{

  namespace details
    {

      template<typename Awaitable> struct __future_awaitable_base;
      template<typename Result> struct __future_coroutine_base;
      template<Future _Future> struct __future_link_coroutine;
    }

  template<details::Future _Future> struct details::__future_link_coroutine: public __future_link_base<_Future>
    {

      typedef __future_link_base<_Future> _parent;
      typedef __future_link_base<_Future>::future_type future_type;

      static inline void create (std::coroutine_handle<> handle, future_type&& future) noexcept
        {

          auto impl = new __future_link_coroutine (handle, std::forward<_Future> (future));
          __future_host_impl->launch (impl);
        }

      virtual void dispatch () noexcept override
        {
          _handle.resume ();
        }

    private:

      std::coroutine_handle<> _handle;

      inline __future_link_coroutine (std::coroutine_handle<> handle, future_type&& future) noexcept:
          _parent (std::forward<_Future> (future)), _handle (handle)
        { }
    };

  template<details::Future _Future> struct details::__future_awaitable_base<_Future>
    {

      typedef _Future future_type;
      typedef unwrap_future_t<_Future> value_type;

      bool await_ready () const noexcept 
        { return _timeout != _future.wait_for (_zero); }

      value_type await_resume ()
        { return std::move (_future).get (); }

      void await_suspend (std::coroutine_handle<> handle)
        {
          using T = std::remove_cvref_t<_Future>;
          details::__future_link_coroutine<T&>::create (handle, _future);
        }

      __future_awaitable_base (const __future_awaitable_base&) = delete;

      inline __future_awaitable_base (_Future&& future) noexcept: _future (std::move (future))
        { }

    private:

      future_type _future;
      static constexpr auto _timeout = std::future_status::timeout;
      static constexpr auto _zero = std::chrono::milliseconds::zero ();
    };

  template<typename Result> struct details::__future_coroutine_base
    {

      std::suspend_always final_suspend () noexcept
        { return { }; }

      std::future<Result> get_return_object ()
        { return _promise.get_future (); }

      std::suspend_never initial_suspend ()
        { return { }; }

      void unhandled_exception ()
        { _promise.set_exception (std::current_exception ()); }


      std::suspend_always await_transform (std::suspend_always)
        { return { }; }

      template<Future _Future>
      static inline __future_awaitable_base<_Future> await_transform (_Future&& future)
        {
          return __future_awaitable_base<_Future> (std::forward<_Future> (future));
        }

      template<Future _Future>
      static inline __future_awaitable_base<std::shared_future<unwrap_future_t<_Future>>> await_transform (_Future& future)
        {
          return __future_awaitable_base<std::shared_future<unwrap_future_t<_Future>>> (future.share ());
        }

    protected:
      std::promise<Result> _promise;
    };
}

namespace std
{

  template<typename T, typename... Args> struct coroutine_traits<std::future<T>, Args ...>
    {

      struct promise_type: public asynclib::details::__future_coroutine_base<T>
        {

          template<std::convertible_to<T> U = T>
          void return_value (U&& value)
            { this->_promise.set_value (std::forward<U> (value)); }
        };
    };

  template<typename... Args> struct coroutine_traits<std::future<void>, Args ...>
    {

      struct promise_type: public asynclib::details::__future_coroutine_base<void>
        {

          void return_void ()
            { this->_promise.set_value (); }
        };
    };
}