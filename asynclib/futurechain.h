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
#include <asynclib/futurepollable.h>

namespace asynclib::details
{

  template<__future Future, typename Functor>
  struct __future_pollable_action: public __future_pollable_base<Future>
    {

      virtual void dispatch () noexcept override
        {
          _action (this->_future);
        }

      static inline void launch (Functor&& action, Future&& future)
          noexcept (std::is_nothrow_constructible_v<__future_pollable_action<Future, Functor>, Functor&&, Future&&>)
        {
          __pollable_host_promise::template launch<__future_pollable_action<Future, Functor>> (std::move (action), std::move (future));
        }

    private:

      Functor _action;

      inline __future_pollable_action (Functor&& action, Future&& future)
          noexcept (std::is_nothrow_constructible_v<__future_pollable_base<Future>, Future&&>
                 && std::is_nothrow_move_constructible_v<Functor>):
          __future_pollable_base<Future> (std::move (future)), _action (std::move (action))
        { }

      friend class __pollable_host_promise;
    };

  template<__future Future, typename R, typename Functor,
           __promise<R> Promise>
  struct __future_pollable_transform: public __future_pollable_base<Future>
    {

      virtual void dispatch () noexcept override
        {
          try
            { _promise.set_value (_action (this->_future)); }
          catch (...)
            { _promise.set_exception (std::current_exception ()); }
        }

      static inline decltype (std::declval<Promise> ().get_future ()) launch (Functor&& action, Future&& future)
          noexcept (std::is_nothrow_constructible_v<__future_pollable_transform<Future, R, Functor, Promise>, Functor&&, Future&&, Promise&&>)
        {

          auto promise = Promise ();
          auto chained = promise.get_future ();

          __pollable_host_promise::template launch<__future_pollable_transform<Future, R, Functor, Promise>> (std::move (action), std::move (future), std::move (promise));
        return chained;
        }

    private:

      Functor _action;
      Promise _promise;

      inline __future_pollable_transform (Functor&& action, Future&& future, Promise&& promise)
          noexcept (std::is_nothrow_constructible_v<__future_pollable_transform<Future, R, Functor, Promise>, Future&&>
                 && std::is_nothrow_move_constructible_v<Functor>
                 && std::is_nothrow_move_constructible_v<Promise>):
          __future_pollable_base<Future> (std::move (future)), _action (std::forward<Functor> (action)), _promise (std::move (promise))
        { }

      friend class __pollable_host_promise;
    };
}

namespace std
{

  template<asynclib::details::__future Future, typename Functor>
    requires (std::is_nothrow_invocable_r_v<void, Functor, std::remove_reference_t<Future>&>)
  static inline void operator>> (Future&& future, Functor&& action)
    {
      return asynclib::details::__future_pollable_action<Future, Functor>::launch (std::forward<Functor> (action), std::forward<Future> (future));
    }

  template<asynclib::details::__future Future, typename Functor,
           typename = std::enable_if_t<std::is_invocable_v<Functor, std::remove_reference_t<Future>&>>>
  static inline auto operator| (Future&& future, Functor&& action)
    {
      using R = std::invoke_result_t<Functor, Future&>;
      return asynclib::details::__future_pollable_transform<Future, R, Functor, std::promise<R>>::launch (std::forward<Functor> (action), std::move (future));
    }
}