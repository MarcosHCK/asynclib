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
#include <asynclib-gio/asyncfunction.h>
#include <asynclib-gio/asyncfunctioncoroutine.h>

namespace asynclib::details
{

  template<__async_function_begin _Begin, __async_function_end _End, typename Functor>
  struct __coroutine_promise_awaiter<__async_function_invocation<_Begin, _End, Functor> &&>
    {

      static inline __async_function_coroutine_awaitable<_Begin, _End, Functor> await_transform (__async_function_invocation<_Begin, _End, Functor>&& invocation) noexcept
        {
          return __async_function_coroutine_awaitable<_Begin, _End, Functor> (std::move (invocation));
        }
    };

  template<__async_function_begin _Begin, __async_function_end _End, typename Functor>
  struct __coroutine_promise_awaiter<__async_function_invocation<_Begin, _End, Functor>>
    {

      static inline __async_function_coroutine_awaitable<_Begin, _End, Functor> await_transform (__async_function_invocation<_Begin, _End, Functor> invocation) noexcept
        {
          return __async_function_coroutine_awaitable<_Begin, _End, Functor> (std::move (invocation));
        }
    };
}