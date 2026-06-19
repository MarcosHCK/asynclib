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
#include <asynclib/coroutine.h>

namespace asynclib::details
{

  template<details::__future _Future>
  struct __future_coroutine_awaitable_link: public __future_host_checker
    {

      static inline void create (std::coroutine_handle<> handle, _Future& future) noexcept
        {

          __future_host_checker::template create<__future_coroutine_awaitable_link> (handle, future);
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

      inline __future_coroutine_awaitable_link (std::coroutine_handle<> handle, _Future& future) noexcept:
          _future (future), _handle (handle)
        { }

      friend struct __future_host_checker;
    };

  template<details::__future _Future>
  struct __future_coroutine_awaitable
    {

      bool await_ready () const noexcept 
        { return _timeout != _future.wait_for (_zero); }

      decltype (std::declval<_Future> ().get ()) await_resume ()
        { return _future.get (); }

      void await_suspend (std::coroutine_handle<> handle)
        {
          using T = std::remove_cvref_t<_Future>;
          __future_coroutine_awaitable_link<T>::create (handle, _future);
        }

      __future_coroutine_awaitable (const __future_coroutine_awaitable&) = delete;

      inline __future_coroutine_awaitable (_Future&& future) noexcept (std::is_nothrow_move_constructible_v<_Future>):
                                                                 _future (std::move (future))
        { }

    private:

      _Future _future;
      static constexpr auto _timeout = std::future_status::timeout;
      static constexpr auto _zero = std::chrono::milliseconds::zero ();
    };

  template<details::__future _Future> struct __coroutine_promise_awaiter<_Future&&>
    {

      static inline __future_coroutine_awaitable<_Future> await_transform (_Future&& future)
        {
          return __future_coroutine_awaitable<_Future> (std::move (future));
        }
    };

  template<details::__future _Future> struct __coroutine_promise_awaiter<_Future>
    {

      static inline __future_coroutine_awaitable<_Future> await_transform (_Future future)
        {
          return __future_coroutine_awaitable<_Future> (std::move (future));
        }
    };

  template<details::__shareable_future _Future> struct __coroutine_promise_awaiter<_Future&>
    {

      static inline __future_coroutine_awaitable<decltype (std::declval<_Future> ().share ())> await_transform (_Future& future)
        {
          return __future_coroutine_awaitable<decltype (std::declval<_Future> ().share ())> (future.share ());
        }
    };
}