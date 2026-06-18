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
#include <gio/gio.h>
#include <tuple>

namespace asynclib::details
{

  template<typename>
  struct __async_function_begin_details
    {
      static constexpr bool invalid = true;
    };

  template<bool _Noexcept, typename... Args>
  struct __async_function_begin_details<void (*) (Args...) noexcept (_Noexcept)>
    {

      using arguments_tuple = std::tuple<Args ...>;

      static constexpr unsigned arguments_n = sizeof...(Args);
      static constexpr bool noexcept_v = _Noexcept;

      template<size_t... Is>
      static auto get_signature_type (std::index_sequence<Is...>)
        -> void (*) (typename std::tuple_element_t<Is, arguments_tuple> ...) noexcept (_Noexcept);

      using signature_type = std::remove_pointer_t<decltype (get_signature_type (std::make_index_sequence<arguments_n - 2> ()))>;

      static constexpr bool invalid = !std::is_convertible_v<typename std::tuple_element_t<arguments_n - 2, arguments_tuple>, GAsyncReadyCallback>
                                   || !std::is_convertible_v<typename std::tuple_element_t<arguments_n - 1, arguments_tuple>, gpointer>;
    };

  template<typename _Function>
  concept __async_function_begin = !__async_function_begin_details<_Function>::invalid;
}