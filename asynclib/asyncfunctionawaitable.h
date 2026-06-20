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
#include <asynclib/error.h>
#include <coroutine>
#include <optional>

namespace asynclib::details
{

  template<__async_function_begin _Begin, __async_function_end _End, typename Functor, typename Return>
    requires (std::is_move_constructible_v<Return>)
  struct __async_function_awaitable_base
    {

      inline constexpr ~__async_function_awaitable_base () noexcept
        {

          if (G_UNLIKELY (NULL != _error))
            g_error_free (_error);
        }

      inline constexpr __async_function_awaitable_base (__async_function_invocation<_Begin, _End, Functor>&& invocation)
          noexcept (std::is_nothrow_move_constructible_v<__async_function_invocation<_Begin, _End, Functor>>):
          _error (nullptr), _invocation (std::move (invocation))
        { }

      inline Return await_resume ()
        {

          if (G_UNLIKELY (NULL != _error))
            glib_error::rethrow (g_steal_pointer (&_error));

        return std::move (_result).value ();
        }

    protected:

      std::optional<Return> _result;

      inline void await_complete (GObject* source_object, GAsyncResult* async_result)
          noexcept (__async_function_end_details<_End>::noexcept_v
                 && std::is_nothrow_move_constructible_v<Return>)
        {

          GError* error = NULL;

          if (Return result = this->_invocation.finish (source_object, async_result, &error); G_UNLIKELY (NULL != error))

            this->_error = error;
          else
            this->_result.emplace (std::move (result));
        }

      GError* _error;
      __async_function_invocation<_Begin, _End, Functor> _invocation;
    };

  template<__async_function_begin _Begin, __async_function_end _End, typename Functor>
  struct __async_function_awaitable_base<_Begin, _End, Functor, void>
    {

      inline constexpr ~__async_function_awaitable_base () noexcept
        {

          if (G_UNLIKELY (NULL != _error))
            g_error_free (_error);
        }

      inline constexpr __async_function_awaitable_base (__async_function_invocation<_Begin, _End, Functor>&& invocation)
          noexcept (std::is_nothrow_move_constructible_v<__async_function_invocation<_Begin, _End, Functor>>):
          _error (nullptr), _invocation (std::move (invocation))
        { }

      inline void await_resume ()
        {

          if (G_UNLIKELY (NULL != _error))
            glib_error::rethrow (g_steal_pointer (&_error));
        }

    protected:

      inline void await_complete (GObject* source_object, GAsyncResult* async_result)
          noexcept (__async_function_end_details<_End>::noexcept_v)
        {

          GError* error = NULL;

          if (this->_invocation.finish (source_object, async_result, &error); G_UNLIKELY (NULL != error))
            this->_error = error;
        }

      GError* _error;
      __async_function_invocation<_Begin, _End, Functor> _invocation;
    };

  template<__async_function_begin _Begin, __async_function_end _End, typename Functor>
  struct __async_function_awaitable:
      public __async_function_awaitable_base<_Begin, _End, Functor, typename __async_function_end_details<_End>::return_type>
    {

      using end_details = __async_function_end_details<_End>;

      inline constexpr __async_function_awaitable (__async_function_invocation<_Begin, _End, Functor>&& invocation)
          noexcept (std::is_nothrow_constructible_v<__async_function_awaitable_base<_Begin, _End, Functor, typename end_details::return_type>, decltype (invocation)&&>):
          __async_function_awaitable_base<_Begin, _End, Functor, typename end_details::return_type> (std::move (invocation))
        { }

      inline bool await_ready () const noexcept 
        { return false; }

      inline void await_suspend (std::coroutine_handle<> handle)
          noexcept (std::is_nothrow_invocable_v<__async_function_invocation<_Begin, _End, Functor>, GAsyncReadyCallback, gpointer>)
        {

          struct Data
            {

              std::coroutine_handle<> handle;
              __async_function_awaitable<_Begin, _End, Functor>& self;

              inline Data (std::coroutine_handle<> _handle, __async_function_awaitable<_Begin, _End, Functor>& _self) noexcept:
                  handle (_handle), self (_self)
                { }
            };

          auto data = new (g_slice_alloc0 (sizeof (Data))) Data (handle, *this);

          this->_invocation ([](GObject* source_object, GAsyncResult* async_result, gpointer user_data)
            {

              auto handle = ((Data*) user_data)->handle;

              ((Data*) user_data)->self.await_complete (source_object, async_result);
              g_slice_free1 (sizeof (Data), (((Data*) user_data)->~Data (), user_data));
              handle.resume ();
            }, data);
        }

    private:

      inline void await_complete (GObject* source_object, GAsyncResult* async_result) noexcept
        {

          if constexpr (end_details::noexcept_v)

            { __async_function_awaitable_base<_Begin, _End, Functor, typename end_details::return_type>::await_complete (source_object, async_result); }
          else try
            { __async_function_awaitable_base<_Begin, _End, Functor, typename end_details::return_type>::await_complete (source_object, async_result); }
          catch (...)
            { g_clear_error (&this->_error); this->_error = asynclib_cpp_error_new (std::current_exception ()); }
        }
    };
}

template<asynclib::details::__async_function_begin _Begin,
         asynclib::details::__async_function_end _End,
         typename Functor>
static inline auto operator co_await (asynclib::details::__async_function_invocation<_Begin, _End, Functor>&& invocation)
  noexcept (std::is_nothrow_constructible_v<asynclib::details::__async_function_awaitable<_Begin, _End, Functor>, decltype (invocation)&&>)
{
  return asynclib::details::__async_function_awaitable<_Begin, _End, Functor> (std::move (invocation));
}