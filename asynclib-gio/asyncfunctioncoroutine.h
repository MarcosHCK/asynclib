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
#include <asynclib-gio/asyncfunctionend.h>
#include <asynclib-gio/gioerror.h>
#include <coroutine>
#include <new>
#include <optional>

namespace asynclib::details
{

  template<__async_function_begin _Begin, __async_function_end _End, typename Functor>
  struct __async_function_coroutine_awaitable_base
    {

      inline constexpr ~__async_function_coroutine_awaitable_base () noexcept
        {

          if (G_UNLIKELY (NULL != _error))
            g_error_free (_error);
        }

      inline constexpr __async_function_coroutine_awaitable_base (__async_function_invocation<_Begin, _End, Functor>&& invocation) noexcept:
          _error (nullptr), _invocation (std::move (invocation))
        { }

    protected:

      inline void await_resume ()
        {

          if (G_UNLIKELY (NULL != _error))
            throw gio_error (g_steal_pointer (&_error));
        }
  
      GError* _error;
      __async_function_invocation<_Begin, _End, Functor> _invocation;
    };

  template<__async_function_begin _Begin, __async_function_end _End, typename Functor, typename _Result>
  struct __async_function_coroutine_awaitable_resumable: public __async_function_coroutine_awaitable_base<_Begin, _End, Functor>
    {

      inline constexpr __async_function_coroutine_awaitable_resumable (__async_function_invocation<_Begin, _End, Functor>&& invocation) noexcept:
          __async_function_coroutine_awaitable_base<_Begin, _End, Functor> (std::move (invocation))
        { }

      inline _Result await_resume ()
        {

          __async_function_coroutine_awaitable_base<_Begin, _End, Functor>::await_resume ();
        return std::move (_result).value ();
        }

    protected:

      std::optional<_Result> _result;

      inline void complete (GObject* source_object, GAsyncResult* async_result) noexcept
        {

          GError* error = NULL;

          if (_Result result = this->_invocation.finish (source_object, async_result, &error); G_UNLIKELY (NULL != error))

            this->_error = error;
          else
            this->_result = result;
        }
    };

  template<__async_function_begin _Begin, __async_function_end _End, typename Functor>
  struct __async_function_coroutine_awaitable_resumable<_Begin, _End, Functor, void>: public __async_function_coroutine_awaitable_base<_Begin, _End, Functor>
    {

      inline constexpr __async_function_coroutine_awaitable_resumable (__async_function_invocation<_Begin, _End, Functor>&& invocation) noexcept:
          __async_function_coroutine_awaitable_base<_Begin, _End, Functor> (std::move (invocation))
        { }

      inline void await_resume ()
        {

          __async_function_coroutine_awaitable_base<_Begin, _End, Functor>::await_resume ();
        }

    protected:

      inline void complete (GObject* source_object, GAsyncResult* async_result) noexcept
        {

          GError* error = NULL;

          if (this->_invocation.finish (source_object, async_result, &error); G_UNLIKELY (NULL != error))
            this->_error = error;
        }
    };

  template<__async_function_begin _Begin, __async_function_end _End, typename Functor>
  struct __async_function_coroutine_awaitable: public __async_function_coroutine_awaitable_resumable<_Begin, _End, Functor,
                                                 typename __async_function_end_details<_End>::return_type>
    {

      inline bool await_ready () const noexcept 
        { return false; }

      inline void await_suspend (std::coroutine_handle<> handle)
          noexcept (std::is_nothrow_constructible_v<std::coroutine_handle<>, std::coroutine_handle<>>
                 && __async_function_begin_details<_Begin>::noexcept_v)
        {

          struct _CompleteData
            {

              __async_function_coroutine_awaitable<_Begin, _End, Functor>* awaitable;
              std::coroutine_handle<> handle;

              inline _CompleteData (__async_function_coroutine_awaitable<_Begin, _End, Functor>* _awaitable, std::coroutine_handle<> _handle) noexcept:
                  awaitable (_awaitable), handle (_handle)
                { }
            };

          this->_invocation ([](GObject* source_object, GAsyncResult* async_result, gpointer user_data)
            {

              auto handle = ((_CompleteData*) user_data)->handle;
              ((_CompleteData*) user_data)->awaitable->complete (source_object, async_result);
              ((_CompleteData*) user_data)->~_CompleteData ();
              g_slice_free1 (sizeof (_CompleteData), user_data);
              handle.resume ();
            }, new (g_slice_alloc0 (sizeof (_CompleteData))) _CompleteData (this, handle));
        }

      inline constexpr __async_function_coroutine_awaitable (__async_function_invocation<_Begin, _End, Functor>&& invocation) noexcept:
          __async_function_coroutine_awaitable_resumable<_Begin, _End, Functor, typename __async_function_end_details<_End>::return_type> (std::move (invocation))
        { }
    };
}