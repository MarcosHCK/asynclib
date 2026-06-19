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

namespace asynclib::details
{

  template<typename Awaitable>
  struct __coroutine_promise_awaiter;

  template<typename _Result, details::__promise<_Result> _Promise> struct __coroutine_promise_base
    {

      inline std::suspend_always final_suspend () noexcept
        { return { }; }

      inline decltype (std::declval<_Promise> ().get_future ()) get_return_object ()
        { return _promise.get_future (); }

      inline std::suspend_never initial_suspend ()
        { return { }; }

      inline void unhandled_exception ()
        { _promise.set_exception (std::current_exception ()); }

      static inline std::suspend_always await_transform (std::suspend_always)
        { return { }; }

      template<typename Awaitable>
      static inline auto await_transform (Awaitable&& awaitable)
        {
          return __coroutine_promise_awaiter<Awaitable>::await_transform (std::forward<Awaitable> (awaitable));
        }

    protected:
      _Promise _promise;
    };
}

namespace std
{

  template<typename T, typename... Args> struct coroutine_traits<std::future<T>, Args ...>
    {

      struct promise_type: public asynclib::details::__coroutine_promise_base<T, std::promise<T>>
        {

          template<std::convertible_to<T> U = T>
          void return_value (U&& value)
            { this->_promise.set_value (std::forward<U> (value)); }
        };
    };

  template<typename... Args> struct coroutine_traits<std::future<void>, Args ...>
    {

      struct promise_type: public asynclib::details::__coroutine_promise_base<void, std::promise<void>>
        {

          void return_void ()
            { this->_promise.set_value (); }
        };
    };
}