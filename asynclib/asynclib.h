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
#include <asynclib/asyncfunction.h>
#include <asynclib/asynctask.h>
#include <asynclib/asynctaskawaitable.h>
#include <asynclib/asynctaskcoroutine.h>
#include <functional>

namespace asynclib
{

  template<details::__async_function_begin _Begin,
           details::__async_function_end _End>
  using async_function = details::__async_function<_Begin, _End>;

  template<details::__async_task_target Return>
  using async_task = details::__async_task<void (*) (GAsyncReadyCallback, gpointer),
                                           Return (*) (GAsyncResult*, GError**),
                                           std::move_only_function<void (GAsyncReadyCallback, gpointer)>>;
}