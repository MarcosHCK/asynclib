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
#include <optional>

namespace asynclib::details
{

  template<typename>
  struct __async_task_tuple_element_details
    {

      static constexpr bool invalid = true;
    };

  template<__async_function_begin _Begin,
           __async_function_end _End,
           typename Functor>
  struct __async_task_tuple_element_details<__async_task<_Begin, _End, Functor>>
    {

      using begin = _Begin;
      using end = _End;

      using begin_details = __async_function_begin_details<_Begin>;
      using end_details = __async_function_end_details<_End>;

      using return_type = end_details::return_type;

      static constexpr bool invalid = false;
    };

  template<typename T>
  concept __async_task_tuple_element = !__async_task_tuple_element_details<T>::invalid;

  template<typename>
  struct __async_task_tuple_details
    {
      static constexpr bool invalid = true;
    };

  template<>
  struct __async_task_tuple_details<std::tuple<>>
    {
      static constexpr bool invalid = true;
    };

  template<__async_task_tuple_element... Args>
    requires ((true == std::is_void_v<typename __async_task_tuple_element_details<Args>::return_type>) && ...)
  struct __async_task_tuple_details<std::tuple<Args ...>>
    {

      static constexpr bool invalid = false;
      static constexpr unsigned element_count = sizeof... (Args);

      using element_details = std::tuple<__async_task_tuple_element_details<Args> ...>;
      using return_type = void;
      using temporary_store = void;
    };

  template<__async_task_tuple_element... Args>
    requires ((false == std::is_void_v<typename __async_task_tuple_element_details<Args>::return_type>) && ...)
  struct __async_task_tuple_details<std::tuple<Args ...>>
    {

      static constexpr bool invalid = false;
      static constexpr unsigned element_count = sizeof... (Args);

      using element_details = std::tuple<__async_task_tuple_element_details<Args> ...>;
      using return_type = std::tuple<typename __async_task_tuple_element_details<Args>::return_type ...>;
      using temporary_store = std::tuple<std::optional<typename __async_task_tuple_element_details<Args>::return_type> ...>;
    };

  template<typename T>
  concept __async_task_tuple = !__async_task_tuple_details<T>::invalid;
}