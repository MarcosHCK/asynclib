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
#include <asynclib/gioerror.h>
#include <asynclib/impl/slice.h>
#include <gio/gio.h>

namespace asynclib::details
{

  template<typename>
  struct __task_complete_function_details
    {
      static constexpr bool invalid = true;
    };

  template<typename Result, typename First>
  struct __task_complete_function_details<Result (First, GAsyncResult*, GError**)>
    {
      using return_value_type = Result;
      using source_object_type = First;
      static constexpr bool invalid = false;
    };

  template<typename Result>
  struct __task_complete_function_details<Result (GAsyncResult*, GError**)>
    {
      using return_value_type = Result;
      using source_object_type = void;
      static constexpr bool invalid = false;
    };

  template<typename return_value_type>
  struct __task_complete_function_implementation_collect
    {

      typedef std::promise<return_value_type> promise_type;

      [[gnu::always_inline]]
      static inline void collect (promise_type* promise, return_value_type result, GError* error) noexcept
        {

          if (G_LIKELY (nullptr == error))

            promise->set_value (std::move (result));
          else
            promise->set_exception (std::make_exception_ptr (gio_error (error)));

        return g_slice_free_<promise_type> (promise);
        }
    };

  template<>
  struct __task_complete_function_implementation_collect<void>
    {

      typedef std::promise<void> promise_type;

      [[gnu::always_inline]]
      static inline void collect (promise_type* promise, GError* error) noexcept
        {

          if (G_LIKELY (nullptr == error))

            promise->set_value ();
          else
            promise->set_exception (std::make_exception_ptr (gio_error (error)));

        return g_slice_free_<promise_type> (promise);
        }
    };

  template<typename return_value_type, typename source_object_type>
  struct __task_complete_function_implementation_complete
    {

      typedef std::promise<return_value_type> promise_type;
      using collector = __task_complete_function_implementation_collect<return_value_type>;

      template<return_value_type (*_Function) (source_object_type, GAsyncResult*, GError**)>
      [[gnu::always_inline]]
      static inline void implementation (GObject* source_object, GAsyncResult* async_result, gpointer user_data) noexcept
        {

          if constexpr (GError* tmperr = nullptr; std::is_same_v<return_value_type, void>)

            { _Function ((source_object_type) source_object, async_result, &tmperr);
              collector::collect ((promise_type*) user_data, tmperr); }
          else
            { auto result = _Function ((source_object_type) source_object, async_result, &tmperr);
              collector::collect ((promise_type*) user_data, std::move (result), tmperr); }
        }
    };

  template<typename return_value_type>
  struct __task_complete_function_implementation_complete<return_value_type, void>
    {

      typedef std::promise<return_value_type> promise_type;
      using collector = __task_complete_function_implementation_collect<return_value_type>;

      template<return_value_type (*_Function) (GAsyncResult*, GError**)>
      [[gnu::always_inline]]
      static inline void implementation (GObject* source_object, GAsyncResult* async_result, gpointer user_data) noexcept
        {

          if constexpr (GError* tmperr = nullptr; std::is_same_v<return_value_type, void>)

            { _Function (async_result, &tmperr);
              collector::collect ((promise_type*) user_data, tmperr); }
          else
            { auto result = _Function (async_result, &tmperr);
              collector::collect ((promise_type*) user_data, std::move (result), tmperr); }
        }
    };

  template<typename return_value_type, typename source_object_type>
  struct __task_complete_function_implementation: public __task_complete_function_implementation_complete<return_value_type, source_object_type>
    {
    };

  template<typename _Function>
    requires (!__task_complete_function_details<_Function>::invalid)
  struct __task_complete_function
    {

      typedef typename __task_complete_function_details<_Function>::return_value_type return_value_type;
      typedef typename __task_complete_function_details<_Function>::source_object_type source_object_type;
      using implementation = __task_complete_function_implementation<return_value_type, source_object_type>;
    };
}