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
#include <asynclib-gio/asyncfunctionbegin.h>
#include <asynclib-gio/asyncfunctionend.h>

namespace asynclib::details
{

  template<__async_function_end _End, typename _Referrer>
  struct __async_function_invocation_base
    {

      _End end;

      inline constexpr __async_function_invocation_base (_End _end) noexcept: end (_end)
        { }

      inline typename __async_function_end_details<_End>::return_type finish (GObject* source_object, GAsyncResult* result, GError** error) const
          noexcept (__async_function_end_details<_End>::noexcept_v)
        {
          return end ((_Referrer) source_object, result, error);
        }
    };

  template<__async_function_end _End>
  struct __async_function_invocation_base<_End, void>
    {

      _End end;

      inline constexpr __async_function_invocation_base (_End _end) noexcept: end (_end)
        { }

      inline __async_function_end_details<_End>::return_type finish (GObject* source_object, GAsyncResult* result, GError** error) const
          noexcept (__async_function_end_details<_End>::noexcept_v)
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

      inline constexpr __async_function_invocation (Functor&& _begin, _End _end) noexcept (std::is_nothrow_move_constructible_v<Functor>):
          __async_function_invocation_base<_End, typename __async_function_end_details<_End>::referrer_type> (_end),
          begin (std::move (_begin))
        { }

      inline constexpr void operator() (GAsyncReadyCallback callback, gpointer user_data) noexcept (__async_function_begin_details<_Begin>::noexcept_v)
        {
          begin (callback, user_data);
        }
    };

  template<__async_function_begin _Begin,
           __async_function_end _End,
           typename>
  struct __async_function_invoker_base;

  template<__async_function_begin _Begin,
           __async_function_end _End,
           bool Noexcept,
           typename... Args>
  struct __async_function_invoker_base<_Begin, _End, void (Args...) noexcept (Noexcept)>
    {

      _Begin begin;
      _End end;

      inline constexpr auto operator() (Args... args) const
        {

          auto lambda = [begin = begin, ...args = args = std::forward<Args> (args)]
                        (GAsyncReadyCallback callback, gpointer user_data) noexcept (Noexcept) -> void
            {
              begin (args..., callback, user_data);
            };

        return __async_function_invocation<_Begin, _End, decltype (lambda)> (std::move (lambda), end);
        }

      inline constexpr __async_function_invoker_base (_Begin _begin, _End _end) noexcept: begin (_begin), end (_end) 
        { }
    };

  template<__async_function_begin _Begin,
           __async_function_end _End>
  struct __async_function: __async_function_invoker_base<_Begin, _End, typename __async_function_begin_details<_Begin>::signature_type>
    {

      using begin_details = __async_function_begin_details<_Begin>;
      using end_details = __async_function_end_details<_End>;

      inline constexpr __async_function (_Begin _begin, _End _end) noexcept:
                                                                   __async_function_invoker_base<_Begin, _End, typename begin_details::signature_type> (_begin, _end)
        { }
    };
}