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
#include <asynclib/asyncfunctioninvocation.h>
#include <functional>
#include <type_traits>

namespace asynclib::details
{

  template<typename T>
  concept __async_task_target = requires ()
    {

      requires std::is_default_constructible_v<T> || std::is_same_v<T, void>;
      requires std::is_move_constructible_v<T> || std::is_same_v<T, void>;
    };

  template<__async_task_target Return>
  using __async_task = __async_function_invocation<void (*) (GAsyncReadyCallback, gpointer),
                                                   Return (*) (GAsyncResult*, GError**),
                                                   std::move_only_function<void (GAsyncReadyCallback, gpointer)>>;
}