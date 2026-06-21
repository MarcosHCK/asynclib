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
#include <asynclib/asynctask.h>
#include <asynclib/error.h>
#include <coroutine>
#include <new>

namespace asynclib::details
{

  struct __async_task_promise_base
    {

      inline ~__async_task_promise_base () noexcept
        {

          if (G_LIKELY (NULL != _task))
            _task = (g_object_unref (_task), nullptr);
        }

      inline std::suspend_never final_suspend () noexcept
        { return { }; }

      inline std::suspend_always initial_suspend () noexcept
        { return { }; }

      inline void unhandled_exception ()
        {
          g_task_return_error (_task, glib_error::to_glib_error (std::current_exception ()));
        }

    protected:
      GTask* _task = nullptr;
    };

  template<__async_task_target Return>
  struct __async_task_promise_completable: public __async_task_promise_base
    {

      template<std::convertible_to<Return> U = Return>
        requires (std::is_constructible_v<Return, U>)
      inline void return_value (U&& value) noexcept (std::is_nothrow_constructible_v<Return, U>)
        {

          if constexpr (sizeof (gpointer) < sizeof (Return))
            g_task_return_pointer (this->_task, new Return (std::forward<U> (value)), notify);

          else if constexpr (gpointer ptr; true)
            g_task_return_pointer (this->_task, (new (&ptr) Return (std::forward<U> (value)), ptr), notify);
        }

    private:

      static void notify (gpointer ptr)
          noexcept (std::is_nothrow_destructible_v<Return>)
        {

          if constexpr (sizeof (gpointer) < sizeof (Return))

            delete (Return*) ptr;
          else
            ((Return*) &ptr)->~Return ();
        }

      static Return steal (gpointer ptr)
          noexcept (std::is_nothrow_move_constructible_v<Return>
                 && std::is_nothrow_destructible_v<Return>)
        {

          if constexpr (sizeof (gpointer) < sizeof (Return))

            return Return (std::move (*(Return*) ptr));
          else
            return Return (std::move (*(Return*) &ptr));
        }

    protected:

      static Return fulfill (GAsyncResult* async_result, GError** error)
          noexcept (std::is_nothrow_move_constructible_v<Return>)
        {

          GError* tmperr = NULL;

          if (auto ptr = g_task_propagate_pointer ((GTask*) async_result, &tmperr); G_UNLIKELY (NULL != tmperr))

            return (g_propagate_error (error, tmperr), Return ());
          else
            { auto result = steal (ptr); return (notify (ptr), result); }
        }
    };

  template<>
  struct __async_task_promise_completable<void>: public __async_task_promise_base
    {

      inline void return_value () noexcept
        {
          g_task_return_pointer (this->_task, NULL, NULL);
        }

    protected:

      static void fulfill (GAsyncResult* async_result, GError** error) noexcept
        {
          g_task_propagate_pointer ((GTask*) async_result, error);
        }
    };
}

namespace std
{

  template<asynclib::details::__async_function_begin _Begin,
           asynclib::details::__async_function_end _End,
           asynclib::details::__async_task_target Functor,
           typename... Args>
  struct coroutine_traits<asynclib::details::__async_task<_Begin, _End, Functor>, Args ...>
    {

      struct promise_type;
    };

  template<asynclib::details::__async_function_begin _Begin,
           asynclib::details::__async_function_end _End,
           asynclib::details::__async_task_target Functor,
           typename... Args>
  struct coroutine_traits<asynclib::details::__async_task<_Begin, _End, Functor>, Args ...>::promise_type:
      public asynclib::details::__async_task_promise_completable<typename asynclib::details::__async_function_end_details<_End>::return_type>
    {

      using return_type = typename asynclib::details::__async_function_end_details<_End>::return_type;

      inline asynclib::details::__async_task<_Begin, _End, Functor> get_return_object () noexcept
        {

          auto begin = [this] (GAsyncReadyCallback callback, gpointer user_data) -> void
            {

              this->_task = g_task_new (NULL, NULL, callback, user_data);
              std::coroutine_handle<coroutine_traits<asynclib::details::__async_task<_Begin, _End, Functor>, Args ...>::promise_type>::from_promise (*this).resume ();
            };
        return asynclib::details::__async_task<_Begin, _End, Functor> (std::move (begin), asynclib::details::__async_task_promise_completable<return_type>::fulfill);
        }
    };
}