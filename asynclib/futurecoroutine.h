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
#include <asynclib/futureawaitable.h>
#include <asynclib/futureconcepts.h>

namespace asynclib::details
{

  template<typename Result, __promise<Result> Promise>
  struct __future_promise_base
    {

      inline std::suspend_always final_suspend () noexcept
        { return { }; }

      inline decltype (std::declval<Promise> ().get_future ()) get_return_object ()
        { return _promise.get_future (); }

      inline std::suspend_never initial_suspend ()
        { return { }; }

      inline void unhandled_exception ()
        { _promise.set_exception (std::current_exception ()); }

    protected:
      Promise _promise;
    };

  template<typename Result, __promise<Result> Promise>
  struct __future_promise_completable: public __future_promise_base<Result, Promise>
    {

      template<std::convertible_to<Result> U = Result>
      void return_value (U&& value)
        { this->_promise.set_value (std::forward<U> (value)); }
    };

  template<__promise<void> Promise>
  struct __future_promise_completable<void, Promise>: public __future_promise_base<void, Promise>
    {

      void return_void ()
        { this->_promise.set_value (); }
    };
}

namespace std
{

  template<typename Result, typename... Args>
  struct coroutine_traits<std::future<Result>, Args ...>
    {
      using promise_type = asynclib::details::__future_promise_completable<Result, std::promise<Result>>;
    };
}