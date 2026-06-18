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
#include <coroutine>

namespace asynclib
{

  namespace details
    {

      template<typename Awaitable> struct __future_awaitable_base;
      template<typename Awaitable> struct __future_awaitable_transform;
      template<typename Result, details::__promise<Result> Promise> struct __future_coroutine_base;
      template<__future _Future> struct __future_link_coroutine;
    }

  template<details::__future _Future> struct details::__future_link_coroutine: public __future_host_checker
    {

      static inline void create (std::coroutine_handle<> handle, _Future& future) noexcept
        {

          __future_host_checker::template create<__future_link_coroutine> (handle, future);
        }

      virtual bool check () const noexcept override
        { return _timeout != _future.wait_for (_zero); }

      virtual void dispatch () noexcept override
        { _handle.resume (); }

    private:

      _Future& _future;
      std::coroutine_handle<> _handle;

      static constexpr auto _timeout = std::future_status::timeout;
      static constexpr auto _zero = std::chrono::milliseconds::zero ();

      inline __future_link_coroutine (std::coroutine_handle<> handle, _Future& future) noexcept:
          _future (future), _handle (handle)
        { }

      friend struct __future_host_checker;
    };

  template<details::__future _Future> struct details::__future_awaitable_base<_Future>
    {

      bool await_ready () const noexcept 
        { return _timeout != _future.wait_for (_zero); }

      decltype (std::declval<_Future> ().get ()) await_resume ()
        { return _future.get (); }

      void await_suspend (std::coroutine_handle<> handle)
        {
          using T = std::remove_cvref_t<_Future>;
          details::__future_link_coroutine<T>::create (handle, _future);
        }

      __future_awaitable_base (const __future_awaitable_base&) = delete;

      inline __future_awaitable_base (_Future&& future) noexcept (std::is_nothrow_move_constructible_v<_Future>):
                                                       _future (std::move (future))
        { }

    private:

      _Future _future;
      static constexpr auto _timeout = std::future_status::timeout;
      static constexpr auto _zero = std::chrono::milliseconds::zero ();
    };

  template<details::__future _Future> struct details::__future_awaitable_transform<_Future&&>
    {

      static inline __future_awaitable_base<_Future> await_transform (_Future&& future)
        {
          return __future_awaitable_base<_Future> (std::move (future));
        }
    };

  template<details::__future _Future> struct details::__future_awaitable_transform<_Future>
    {

      static inline __future_awaitable_base<_Future> await_transform (_Future future)
        {
          return __future_awaitable_base<_Future> (std::move (future));
        }
    };

  template<details::__shareable_future _Future> struct details::__future_awaitable_transform<_Future&>
    {

      static inline __future_awaitable_base<decltype (std::declval<_Future> ().share ())> await_transform (_Future& future)
        {
          return __future_awaitable_base<decltype (std::declval<_Future> ().share ())> (future.share ());
        }
    };

  template<typename _Result, details::__promise<_Result> _Promise> struct details::__future_coroutine_base
    {

      std::suspend_always final_suspend () noexcept
        { return { }; }

      decltype (std::declval<_Promise> ().get_future ()) get_return_object ()
        { return _promise.get_future (); }

      std::suspend_never initial_suspend ()
        { return { }; }

      void unhandled_exception ()
        { _promise.set_exception (std::current_exception ()); }

      std::suspend_always await_transform (std::suspend_always)
        { return { }; }

      template<typename Awaitable>
      static inline auto await_transform (Awaitable&& awaitable)
        {
          return __future_awaitable_transform<Awaitable>::await_transform (std::forward<Awaitable> (awaitable));
        }

    protected:
      _Promise _promise;
    };
}

namespace std
{

  template<typename T, typename... Args> struct coroutine_traits<std::future<T>, Args ...>
    {

      struct promise_type: public asynclib::details::__future_coroutine_base<T, std::promise<T>>
        {

          template<std::convertible_to<T> U = T>
          void return_value (U&& value)
            { this->_promise.set_value (std::forward<U> (value)); }
        };
    };

  template<typename... Args> struct coroutine_traits<std::future<void>, Args ...>
    {

      struct promise_type: public asynclib::details::__future_coroutine_base<void, std::promise<void>>
        {

          void return_void ()
            { this->_promise.set_value (); }
        };
    };
}