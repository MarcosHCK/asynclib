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
#include <asynclib/asyncfunctionbegin.h>
#include <asynclib/asyncfunctionend.h>

namespace asynclib::details
{

  template<__async_function_end _End, typename Referrer>
  struct __async_function_invocation_base
    {

      _End end;
      using end_details = __async_function_end_details<_End>;

      inline constexpr __async_function_invocation_base (_End _end) noexcept: end (_end)
        { }

      inline typename end_details::return_type finish (GObject* source_object, GAsyncResult* result, GError** error) const noexcept (end_details::noexcept_v)
        {
          return end ((Referrer) source_object, result, error);
        }
    };

  template<__async_function_end _End>
  struct __async_function_invocation_base<_End, void>
    {

      _End end;
      using end_details = __async_function_end_details<_End>;

      inline constexpr __async_function_invocation_base (_End _end) noexcept: end (_end)
        { }

      inline typename end_details::return_type finish (GObject* source_object, GAsyncResult* result, GError** error) const noexcept (end_details::noexcept_v)
        {
          return end (result, error);
        }
    };

  template<__async_function_begin _Begin,
           __async_function_end _End,
           typename Functor>
  struct __async_function_invocation: public __async_function_invocation_base<_End, typename __async_function_end_details<_End>::referrer_type>
    {

      Functor begin;
      using end_details = __async_function_end_details<_End>;

      inline constexpr __async_function_invocation (Functor&& _begin, _End _end) noexcept (std::is_nothrow_move_constructible_v<Functor>):
          __async_function_invocation_base<_End, typename end_details::referrer_type> (_end),
          begin (std::move (_begin))
        { }

      inline constexpr void operator() (GAsyncReadyCallback callback, gpointer user_data) noexcept (std::is_nothrow_invocable_v<Functor, GAsyncReadyCallback, gpointer>)
        {
          begin (callback, user_data);
        }
    };
}