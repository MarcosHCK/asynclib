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
#include <asynclib-gio/asyncfunctionbase.h>

namespace asynclib
{

  template<details::__async_function_begin auto _Begin,
           details::__async_function_end auto _End>
  struct async_function: details::__async_function<decltype (_Begin), decltype (_End)>
    {

      inline constexpr async_function () noexcept:
                                         details::__async_function<decltype (_Begin), decltype (_End)> (_Begin, _End)
        { }
    };
}