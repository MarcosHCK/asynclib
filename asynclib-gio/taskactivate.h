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
#include <asynclib-gio/slice.h>
#include <gio/gio.h>
#include <tuple>

namespace asynclib::details
{

  template<typename>
  struct __task_activate_function_details
    {
      static constexpr bool invalid = true;
    };

  template<typename... Args>
  struct __task_activate_function_details<void (Args...)>
    {

      using arguments_tuple_type = std::tuple<Args ...>;
      static constexpr unsigned N = sizeof...(Args);

      static constexpr bool invalid = !std::is_convertible_v<typename std::tuple_element_t<N - 2, arguments_tuple_type>, GAsyncReadyCallback>
                                   || !std::is_convertible_v<typename std::tuple_element_t<N - 1, arguments_tuple_type>, gpointer>;
    };

  template<typename>
  struct __task_activate_function_extractor;

  template<typename... Args>
  struct __task_activate_function_extractor<void (Args...)>
    {

      using tuple_type = std::tuple<Args ...>;
      static constexpr unsigned N = sizeof...(Args);

      template<size_t... Is>
      static auto get_signature_type (std::index_sequence<Is...>)
        -> void (*) (typename std::tuple_element_t<Is, tuple_type> ...);

      using signature_type = std::remove_pointer_t<decltype (get_signature_type (std::make_index_sequence<N - 2> ()))>;
    };

  template<typename Result,
           __promise<Result> promise_type,
           typename...>
  struct __task_activate_function_implementation;

  template<typename Result,
           __promise<Result> promise_type,
           typename... Args>
  struct __task_activate_function_implementation<Result, promise_type, void (Args...)>
    {

      template<void (*_Activate) (Args..., GAsyncReadyCallback, gpointer),
               void (*_Complete) (GObject*, GAsyncResult*, gpointer)>
      [[gnu::always_inline]]
      static inline std::future<Result> implementation (Args... args) noexcept
        {

          auto promise = g_slice_new_<promise_type> ();
          _Activate (std::forward<Args> (args) ..., _Complete, (gpointer) promise);
        return promise->get_future ();
        }
    };

  template<typename _Function>
  concept __task_activate_function_ = requires ()
    {

      requires !__task_activate_function_details<_Function>::invalid;
    };

  template<__task_activate_function_ _Function,
           typename _Result,
           __promise<_Result> promise_type>
  struct __task_activate_function
    {

      typedef typename __task_activate_function_extractor<_Function>::signature_type signature_type;
      using implementation = __task_activate_function_implementation<_Result, promise_type, signature_type>;
    };
}