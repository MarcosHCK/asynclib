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
#include <asynclib/coroutine.h>
#include <asynclib-gio/asyncfunction.h>
#include <asynclib-gio/asyncfunctioncoroutine.h>
#include <asynclib-gio/cpperror.h>
#include <asynclib-gio/gioerror.h>
#include <coroutine>
#include <functional>

namespace asynclib
{

  namespace details
    {

      struct __coroutine_async_function_base;
      template<typename Return>
      struct __coroutine_async_function_base_completable;
    }

  template<typename Return>
  using async_task = details::__async_function_invocation<void (*) (GAsyncReadyCallback, gpointer),
                                                          Return (*) (GAsyncResult*, GError**),
                                                          std::move_only_function<void (GAsyncReadyCallback, gpointer)>>;

  struct details::__coroutine_async_function_base
    {

      inline std::suspend_always final_suspend () noexcept
        { return { }; }

      inline std::suspend_always initial_suspend () noexcept
        { return { }; }

      inline void unhandled_exception ()
        {

          try
            { std::rethrow_exception (std::current_exception ()); }
          catch (asynclib::gio_error& error)
            { g_task_return_error (_task, error.steal ()); }
          catch (...)
            { g_task_return_error (_task, asynclib_cpp_error_new (std::current_exception ())); }
        }

      inline ~__coroutine_async_function_base () noexcept
        {

          if (G_LIKELY (NULL != _task))
            _task = (g_object_unref (_task), nullptr);
        }

    protected:

      GTask* _task = nullptr;
    };

  template<typename Return>
  struct details::__coroutine_async_function_base_completable: public __coroutine_async_function_base
    {

      template<std::convertible_to<Return> U = Return>
      inline void return_value (U&& value)
        {

          auto val = new Return (std::forward<U> (value));
        return g_task_return_pointer (this->_task, val, notify);
        }

    private:

      static void notify (gpointer ptr) noexcept
        {
          delete (Return*) ptr;
        }

    protected:

      static Return fulfill (GAsyncResult* async_result, GError** error)
          noexcept (std::is_nothrow_move_constructible_v<Return>)
        {

          GError* tmperr = NULL;

          if (auto ptr = g_task_propagate_pointer ((GTask*) async_result, &tmperr); G_UNLIKELY (NULL != tmperr))

            return (g_propagate_error (error, tmperr), Return ());
          else
            { auto result = Return (std::move (*(Return*) ptr)); return (notify (ptr), result); }
        }
    };
}

namespace std
{

  template<typename Return, typename... Args>
  struct coroutine_traits<asynclib::async_task<Return>, Args ...>
    {

      struct promise_type;
    };

  template<typename Return, typename... Args>
  struct coroutine_traits<asynclib::async_task<Return>, Args ...>::promise_type:
      public asynclib::details::__coroutine_async_function_base_completable<Return>
    {

      inline asynclib::async_task<Return> get_return_object () noexcept
        {

          auto begin = [this] (GAsyncReadyCallback callback, gpointer user_data) -> void
            {

              this->_task = g_task_new (NULL, NULL, callback, user_data);
              std::coroutine_handle<coroutine_traits<asynclib::async_task<Return>, Args ...>::promise_type>::from_promise (*this).resume ();
            };
        return asynclib::async_task<Return> (std::move (begin), asynclib::details::__coroutine_async_function_base_completable<Return>::fulfill);
        }

      template<typename Awaitable>
      static inline auto await_transform (Awaitable&& awaitable)
        {
          return asynclib::details::__coroutine_promise_awaiter<Awaitable>::await_transform (std::forward<Awaitable> (awaitable));
        }
    };
}