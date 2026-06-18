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

namespace asynclib::details
{

  template<typename>
  struct __async_function_end_details
    {
      static constexpr bool invalid = true;
    };

  template<bool _Noexcept, typename _Result, typename _Referrer>
  struct __async_function_end_details<_Result (*) (_Referrer, GAsyncResult*, GError**) noexcept (_Noexcept)>
    {

      using referrer_type = _Referrer;
      using return_type = _Result;

      static constexpr bool noexcept_v = _Noexcept;
      static constexpr bool invalid = false;
    };

  template<bool _Noexcept, typename _Result>
  struct __async_function_end_details<_Result (*) (GAsyncResult*, GError**) noexcept (_Noexcept)>
    {

      using referrer_type = void;
      using return_type = _Result;

      static constexpr bool noexcept_v = _Noexcept;
      static constexpr bool invalid = false;
    };

  template<typename _Function>
  concept __async_function_end = !__async_function_end_details<_Function>::invalid;
}