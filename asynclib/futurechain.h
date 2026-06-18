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
#include <functional>

namespace asynclib
{

  namespace details
    {

      template<__future _Future> struct __future_link_action;
      template<__future _Future, typename R> struct __future_link_result;
    }

  template<details::__future _Future> struct details::__future_link_action: public __future_link_base<_Future>
    {

      typedef __future_link_base<_Future> _parent;
      typedef std::move_only_function<void (_Future&)> action_type;

      static inline void create (action_type&& action, _Future&& future)
        {

          __future_host_checker::template create<__future_link_action> (std::move (action), std::forward<_Future> (future));
        }

      virtual void dispatch () noexcept override
        {
          return _action (this->_future);
        }

    private:

      action_type _action;

      inline __future_link_action (action_type&& action, _Future&& future)
          noexcept (std::is_nothrow_constructible_v<_parent, _Future&&> &&
                    std::is_nothrow_move_constructible_v<action_type>):
          _parent (std::forward<_Future> (future)), _action (std::move (action))
        { }

      friend struct __future_host_checker;
    };

  template<details::__future _Future, typename R> struct details::__future_link_result: public __future_link_base<_Future>
    {

      typedef __future_link_base<_Future> _parent;
      typedef std::move_only_function<R (_Future&)> action_type;

      static inline std::future<R> create (action_type&& action, _Future&& future)
        {

          auto promise = std::promise<R> ();
          auto chained = promise.get_future ();

          __future_host_checker::template create<__future_link_result> (std::move (action), std::forward<_Future> (future), std::move (promise));
        return chained;
        }

      virtual void dispatch () noexcept override
        {
          try
            { _promise.set_value (_action (this->_future)); }
          catch (...)
            { _promise.set_exception (std::current_exception ()); }
        }

    private:

      action_type _action;
      std::promise<R> _promise;

      inline __future_link_result (action_type&& action, _Future&& future, std::promise<R>&& promise)
          noexcept (std::is_nothrow_constructible_v<_parent, _Future&&> &&
                    std::is_nothrow_move_constructible_v<action_type> &&
                    std::is_nothrow_move_constructible_v<std::promise<R>>):
          _parent (std::move (future)), _action (std::move (action)), _promise (std::move (promise))
        { }

      friend struct __future_host_checker;
    };
}

namespace std
{

  template<asynclib::details::__future _Future, typename Fn,
           typename = std::enable_if_t<std::is_nothrow_invocable_r_v<void, Fn, std::remove_reference_t<_Future>&>>>
  static inline void operator>> (_Future&& future, Fn&& action)
    {
      auto f = std::move_only_function<void (_Future&)> (std::forward<Fn> (action));
      return asynclib::details::__future_link_action<_Future>::create (std::move (f), std::forward<_Future> (future));
    }

  template<asynclib::details::__future _Future, typename Fn,
           typename = std::enable_if_t<std::is_invocable_v<Fn, std::remove_reference_t<_Future>&>>>
  static inline std::future<std::invoke_result_t<Fn, _Future&>> operator| (_Future&& future, Fn&& action)
    {
      using R = std::invoke_result_t<Fn, _Future&>;
      auto f = std::move_only_function<R (_Future&)> (std::forward<Fn> (action));
      return asynclib::details::__future_link_result<_Future, R>::create (std::move (f), std::forward<_Future> (future));
    }
}