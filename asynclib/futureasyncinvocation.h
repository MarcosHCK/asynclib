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
#include <asynclib/asyncfunctioninvocation.h>
#include <asynclib/futureconcepts.h>
#include <asynclib/error.h>

namespace asynclib::details
{

  template<template<typename> class Promise,
           __async_function_end _End,
           typename Return>
    requires (std::is_default_constructible_v<Return>
           && details::__promise<Promise<typename details::__async_function_end_details<_End>::return_type>,
                                         typename details::__async_function_end_details<_End>::return_type>)
  struct __future_async_invocation_data_base:
      public details::__async_function_invocation_base<_End,
        typename details::__async_function_end_details<_End>::referrer_type>
    {

      Promise<typename details::__async_function_end_details<_End>::return_type> promise;

      inline __future_async_invocation_data_base (_End _end)
          noexcept (std::is_nothrow_default_constructible_v<Promise<typename details::__async_function_end_details<_End>::return_type>>):
          details::__async_function_invocation_base<_End, typename details::__async_function_end_details<_End>::referrer_type> (_end),
          promise ()
        { }

      static inline void complete (GObject* source_object, GAsyncResult* async_result, gpointer user_data) noexcept
        {

          GError* tmperr = NULL;
          using data = __future_async_invocation_data_base<Promise, _End, Return>;

          if (Return result = ((data*) user_data)->finish (source_object, async_result, &tmperr); G_LIKELY (NULL == tmperr))

            ((data*) user_data)->promise.set_value (result);
          else
            ((data*) user_data)->promise.set_exception (from_glib_error (tmperr));
        }
    };

  template<template<typename> class Promise,
           __async_function_end _End>
    requires (details::__promise<Promise<typename details::__async_function_end_details<_End>::return_type>,
                                         typename details::__async_function_end_details<_End>::return_type>)
  struct __future_async_invocation_data_base<Promise, _End, void>:
      public details::__async_function_invocation_base<_End,
        typename details::__async_function_end_details<_End>::referrer_type>
    {

      Promise<typename details::__async_function_end_details<_End>::return_type> promise;

      inline __future_async_invocation_data_base (_End _end)
          noexcept (std::is_nothrow_default_constructible_v<Promise<typename details::__async_function_end_details<_End>::return_type>>):
          details::__async_function_invocation_base<_End, typename details::__async_function_end_details<_End>::referrer_type> (_end),
          promise ()
        { }

      static inline void complete (GObject* source_object, GAsyncResult* async_result, gpointer user_data) noexcept
        {

          GError* tmperr = NULL;
          using data = __future_async_invocation_data_base<Promise, _End, void>;

          if (((data*) user_data)->finish (source_object, async_result, &tmperr); G_LIKELY (NULL == tmperr))

            ((data*) user_data)->promise.set_value ();
          else
            ((data*) user_data)->promise.set_exception (from_glib_error (tmperr));
        }
    };
}

namespace asynclib
{

  template<template<typename> class Promise = std::promise,
           details::__async_function_begin _Begin,
           details::__async_function_end _End,
           typename Functor>
    requires (details::__promise<Promise<typename details::__async_function_end_details<_End>::return_type>,
                                         typename details::__async_function_end_details<_End>::return_type>)
  static inline auto to_future (details::__async_function_invocation<_Begin, _End, Functor>&& invocation)
      noexcept (std::is_nothrow_default_constructible_v<Promise<typename details::__async_function_end_details<_End>::return_type>>
             && std::is_nothrow_invocable_v<decltype (std::declval<Promise<typename details::__async_function_end_details<_End>::return_type>> ().get_future ())>
             && details::__async_function_begin_details<_Begin>::noexcept_v)
    {

      auto data = new details::__future_async_invocation_data_base<Promise, _End, typename details::__async_function_end_details<_End>::return_type> (invocation.end);
      auto future = data->promise.get_future ();

      invocation (details::__future_async_invocation_data_base<Promise, _End, typename details::__async_function_end_details<_End>::return_type>::complete, data);
    return future;
    }
}