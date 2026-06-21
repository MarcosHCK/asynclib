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
#include <asynclib/asynctask.h>

namespace asynclib::details
{

  template<__async_function_begin _Begin,
           __async_function_end _End,
           typename>
  struct __async_function_base;

  template<__async_function_begin _Begin,
           __async_function_end _End,
           bool Noexcept,
           typename... Args>
  struct __async_function_base<_Begin, _End, void (Args ...) noexcept (Noexcept)>
    {

      _Begin begin;
      using begin_details = __async_function_begin_details<_Begin>;
      _End end;
      using end_details = __async_function_begin_details<_Begin>;

      inline constexpr auto operator() (Args... args) const
        {

          auto lambda = [begin = begin, ...args = args = std::forward<Args> (args)]
                        (GAsyncReadyCallback callback, gpointer user_data) noexcept (Noexcept) -> void
            {
              begin (args..., callback, user_data);
            };

        return __async_task<_Begin, _End, decltype (lambda)> (std::move (lambda), end);
        }

      inline constexpr __async_function_base (_Begin _begin, _End _end) noexcept: begin (_begin), end (_end) 
        { }
    };

  template<__async_function_begin _Begin,
           __async_function_end _End>
  struct __async_function: __async_function_base<_Begin, _End, typename __async_function_begin_details<_Begin>::signature_type>
    {

      using begin_details = __async_function_begin_details<_Begin>;
      using end_details = __async_function_end_details<_End>;

      inline constexpr __async_function (_Begin _begin, _End _end) noexcept: __async_function_base<_Begin, _End, typename begin_details::signature_type> (_begin, _end)
        { }
    };
}