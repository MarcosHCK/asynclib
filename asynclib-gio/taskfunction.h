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
#include <gio/gio.h>

namespace asynclib::details
{

  template<auto _Activate, auto _Complete,
           __promise<typename __task_complete_function_details<std::remove_pointer_t<decltype (_Complete)>>::return_value_type> promise_type,
           typename>
    requires (! __task_activate_function_details<std::remove_pointer_t<decltype (_Activate)>>::invalid
           && ! __task_complete_function_details<std::remove_pointer_t<decltype (_Complete)>>::invalid)
  struct __task_function_implementation;

  template<auto _Activate, auto _Complete,
           __promise<typename __task_complete_function_details<std::remove_pointer_t<decltype (_Complete)>>::return_value_type> promise_type,
           typename... Args>
    requires (! __task_activate_function_details<std::remove_pointer_t<decltype (_Activate)>>::invalid
           && ! __task_complete_function_details<std::remove_pointer_t<decltype (_Complete)>>::invalid)
  struct __task_function_implementation<_Activate, _Complete, promise_type, void (Args...)>
    {

      typedef __task_complete_function<std::remove_pointer_t<decltype (_Complete)>, promise_type> complete_function;
      typedef typename complete_function::return_value_type return_value_type;
      typedef __task_activate_function<std::remove_pointer_t<decltype (_Activate)>, return_value_type, promise_type> activate_function;

      static inline std::future<return_value_type> operator() (Args... args) noexcept
        {

        return activate_function::implementation::template implementation<_Activate,
               complete_function::implementation::template implementation<_Complete>> (std::forward<Args> (args) ...);
        }
    };

  template<auto _Activate, auto _Complete,
           __promise<typename __task_complete_function_details<std::remove_pointer_t<decltype (_Complete)>>::return_value_type> promise_type>
    requires (! __task_activate_function_details<std::remove_pointer_t<decltype (_Activate)>>::invalid
           && ! __task_complete_function_details<std::remove_pointer_t<decltype (_Complete)>>::invalid)
  struct __task_function: public __task_function_implementation<_Activate, _Complete, promise_type,
                            typename __task_activate_function_extractor<std::remove_pointer_t<decltype (_Activate)>>::signature_type>
    {

      typedef __task_complete_function<std::remove_pointer_t<decltype (_Complete)>, promise_type> complete_function;

      typedef typename complete_function::return_value_type return_value_type;
      typedef typename complete_function::source_object_type source_object_type;

      typedef __task_activate_function<std::remove_pointer_t<decltype (_Activate)>, return_value_type, promise_type> activate_function;

      typedef typename activate_function::signature_type signature_type;
    };
}