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
#include <asynclib/futureconcepts.h>
#include <asynclib/pollablehost.h>
#include <coroutine>

namespace asynclib::details
{

  template<__future Future>
  struct __future_awaitable_awaker: public __pollable_host_promise
    {

      virtual bool check () const noexcept override
        {
          return _timeout != _future.wait_for (_zero);
        }

      virtual void dispatch () noexcept override
        { _handle.resume (); }

      static inline void launch (std::coroutine_handle<> handle, Future& future) noexcept
        {

          __pollable_host_promise::template launch<__future_awaitable_awaker> (handle, future);
        }

    private:

      Future& _future;
      std::coroutine_handle<> _handle;

      static constexpr auto _timeout = std::future_status::timeout;
      static constexpr auto _zero = std::chrono::milliseconds::zero ();

      inline __future_awaitable_awaker (std::coroutine_handle<> handle, Future& future) noexcept: _future (future), _handle (handle)
        { }

      friend class __pollable_host_promise;
    };

  template<__future Future>
  struct __future_awaitable
    {

      inline bool await_ready () const noexcept
        { return _timeout != _future.wait_for (_zero); }

      inline decltype (std::declval<Future> ().get ()) await_resume ()
        { return _future.get (); }

      inline void await_suspend (std::coroutine_handle<> handle)
        {

          using T = std::remove_cvref_t<Future>;
          __future_awaitable_awaker<T>::launch (handle, _future);
        }

      inline __future_awaitable (Future&& future) noexcept (std::is_nothrow_move_constructible_v<Future>):
                                                 _future (std::move (future))
        { }

    private:

      Future _future;
      static constexpr auto _timeout = std::future_status::timeout;
      static constexpr auto _zero = std::chrono::milliseconds::zero ();
    };
}

template<asynclib::details::__future Future>
static inline auto operator co_await (Future&& future) noexcept (std::is_nothrow_move_constructible_v<Future>)
  {
    return asynclib::details::__future_awaitable (std::move (future));
  }

template<asynclib::details::__shareable_future Future>
static inline auto operator co_await (Future& future) noexcept (std::is_nothrow_move_constructible_v<decltype (std::declval<Future> ().share ())>
                                                             && asynclib::details::__shareable_future_nothrow<Future>)
  {
    return asynclib::details::__future_awaitable (future.share ());
  }