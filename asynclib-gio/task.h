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
#include <asynclib-gio/taskactivate.h>
#include <asynclib-gio/taskcomplete.h>
#include <asynclib-gio/taskfunction.h>

namespace asynclib
{

  template<auto _Activate, auto _Complete>
    requires (! details::__task_activate_function_details<std::remove_pointer_t<decltype (_Activate)>>::invalid
           && ! details::__task_complete_function_details<std::remove_pointer_t<decltype (_Complete)>>::invalid)
  struct gio_promise: public details::__task_function<_Activate, _Complete,
                        std::promise<typename details::__task_complete_function_details<std::remove_pointer_t<decltype (_Complete)>>::return_value_type>>
    {
    };
}