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

namespace std
{

  template<asynclib::details::Future _Future, typename Fn,
           typename = std::enable_if_t<std::is_nothrow_invocable_r_v<void, Fn, std::remove_reference_t<_Future>&>>>
  static inline void operator>> (_Future&& future, Fn&& action)
    {
      auto f = std::move_only_function<void (_Future&)> (std::forward<Fn> (action));
      return asynclib::details::__future_link_action<_Future>::create (std::move (f), std::forward<_Future> (future));
    }

  template<asynclib::details::Future _Future, typename Fn,
           typename = std::enable_if_t<std::is_invocable_v<Fn, std::remove_reference_t<_Future>&>>>
  static inline std::future<std::invoke_result_t<Fn, _Future&>> operator| (_Future&& future, Fn&& action)
    {
      using R = std::invoke_result_t<Fn, _Future&>;
      auto f = std::move_only_function<R (_Future&)> (std::forward<Fn> (action));
      return asynclib::details::__future_link_result<_Future, R>::create (std::move (f), std::forward<_Future> (future));
    }
}