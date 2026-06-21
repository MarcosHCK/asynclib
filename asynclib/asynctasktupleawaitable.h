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
#include <asynclib/aggregateerror.h>
#include <asynclib/asynctasktuple.h>
#include <asynclib/gliberror.h>
#include <coroutine>
#include <optional>
#include <utility>

namespace asynclib::details
{

  template<__async_task_tuple Tuple, typename Return>
  struct __async_task_tuple_awaitable_base
    {

      inline __async_task_tuple_awaitable_base (Tuple&& tuple) noexcept (std::is_nothrow_move_constructible_v<Tuple>):
          _tuple (std::move (tuple))
        { }

    protected:

      Tuple _tuple;
      Return _return {};

      using element_details = __async_task_tuple_details<Tuple>::element_details;

      template<unsigned I>
      inline void await_complete (GObject* source_object, GAsyncResult* async_result, GError** error) noexcept
        {

          if constexpr (std::tuple_element_t<I, element_details>::end_details::noexcept_v)

            { std::get<I> (_return).emplace (std::get<I> (_tuple).finish (source_object, async_result, error)); }
          else try
            { std::get<I> (_return).emplace (std::get<I> (_tuple).finish (source_object, async_result, error)); }
          catch (...)
            { g_clear_error (error); g_propagate_error (error, glib_error::to_glib_error (std::current_exception ())); }
        }

      inline auto await_resume_ () noexcept
        {
          return unwrap_result (std::move (_return));
        }

      template<typename... Args>
      static inline std::tuple<Args ...> unwrap_result (std::tuple<std::optional<Args> ...>&& _return) noexcept
        {

          return std::apply ([](auto&&... fields) { return std::tuple<Args ...> (std::forward<decltype (fields)> (fields).value () ...); },
                             std::forward<std::tuple<std::optional<Args> ...>> (_return));
        }
    };

  template<__async_task_tuple Tuple>
  struct __async_task_tuple_awaitable_base<Tuple, void>
    {

      inline __async_task_tuple_awaitable_base (Tuple&& tuple) noexcept (std::is_nothrow_move_constructible_v<Tuple>):
          _tuple (std::move (tuple))
        { }

    protected:

      Tuple _tuple;

      using element_details = __async_task_tuple_details<Tuple>::element_details;

      template<unsigned I>
      inline void await_complete (GObject* source_object, GAsyncResult* async_result, GError** error) noexcept
        {

          if constexpr (std::tuple_element_t<I, element_details>::end_details::noexcept_v)

            { std::get<I> (_tuple).finish (source_object, async_result, error); }
          else try
            { std::get<I> (_tuple).finish (source_object, async_result, error); }
          catch (...)
            { g_clear_error (error); g_propagate_error (error, glib_error::to_glib_error (std::current_exception ())); }
        }

      inline void await_resume_ () noexcept
        {
        }
    };

  struct __async_task_tuple_awaitable_base_type_erased
    {

      struct _invoke_source
        {

          GSource parent;
          GAsyncResult* async_result;
          GObject* source_object;
          gpointer user_data;
        };

      static GSource* invoke_source_new (GObject* source_object, GAsyncResult* async_result, gpointer user_data) noexcept;
    };

  template<__async_task_tuple Tuple>
  struct __async_task_tuple_awaitable:
      public __async_task_tuple_awaitable_base_type_erased,
      public __async_task_tuple_awaitable_base<Tuple, typename __async_task_tuple_details<Tuple>::temporary_store>
    {

      inline __async_task_tuple_awaitable (Tuple&& tuple)
          noexcept (std::is_nothrow_constructible_v<__async_task_tuple_awaitable_base<Tuple, temporary_store>, Tuple&&>):
          __async_task_tuple_awaitable_base<Tuple, temporary_store> (std::move (tuple)),
          _main_context (g_main_context_ref_thread_default ())
        {

          for (unsigned i = 0; i < __async_task_tuple_details<Tuple>::element_count; ++i)
            _errors [i] = NULL;
        }

      inline ~__async_task_tuple_awaitable () noexcept
        {

          destroy_errors ();
          _main_context = (g_main_context_unref (_main_context), nullptr);
        }

      inline bool await_ready () const noexcept 
        { return false; }

      inline auto await_resume ()
        {

          throw_errors ();
        return this->await_resume_ ();
        }

      inline void await_suspend (std::coroutine_handle<> handle)
        {

          auto data = new (g_slice_alloc0 (sizeof (callback_data))) callback_data (handle, *this);
          await_begin (data, std::make_integer_sequence<unsigned, __async_task_tuple_details<Tuple>::element_count> ());
        }

    private:

      struct callback_data
        {

          std::coroutine_handle<> handle;
          __async_task_tuple_awaitable<Tuple>& self;

          inline callback_data (std::coroutine_handle<> _handle, __async_task_tuple_awaitable<Tuple>& _self) noexcept:
              handle (_handle), self (_self)
            { }
        };

      GError* _errors [__async_task_tuple_details<Tuple>::element_count];
      GMainContext* _main_context;
      unsigned _pending = __async_task_tuple_details<Tuple>::element_count;

      using element_details = __async_task_tuple_details<Tuple>::element_details;
      using temporary_store = __async_task_tuple_details<Tuple>::temporary_store;

      template<unsigned... Is>
      inline void await_begin (callback_data* data, std::integer_sequence<unsigned, Is ...> const&)
        {
          (std::get<Is> (this->_tuple) (await_complete_callback<Is>, (gpointer) data), ...);
        }

      template<unsigned I>
      static inline void await_complete_callback (GObject* source_object, GAsyncResult* async_result, gpointer user_data) noexcept
        {

          auto& self = ((callback_data*) user_data)->self;

          if (! g_main_context_is_owner (self._main_context))
            {

              auto source = self.invoke_source_new (source_object, async_result, user_data);

              g_source_set_callback (source, [](gpointer user_data) noexcept -> gboolean
                {

                  auto async_result = ((_invoke_source*) user_data)->async_result;
                  auto source_object = ((_invoke_source*) user_data)->source_object;

                  await_complete_callback<I> (source_object, async_result, ((_invoke_source*) user_data)->user_data);
                  return G_SOURCE_REMOVE;
                }, (gpointer) source, NULL);

              g_source_attach (source, self._main_context);
            return g_source_unref (source);
            }

          self.template await_complete<I> (source_object, async_result, &self._errors [I]);

          if (auto handle = ((callback_data*) user_data)->handle; 0 == (--self._pending))
            {
              g_slice_free1 (sizeof (callback_data), (((callback_data*) user_data)->~callback_data (), user_data));
              handle.resume ();
            }
        }

      template<unsigned I>
      static gboolean await_complete_source (gpointer user_data)
        {

          auto async_result = ((_invoke_source*) user_data)->async_result;
          auto source_object = ((_invoke_source*) user_data)->source_object;

          await_complete_callback<I> (source_object, async_result, ((_invoke_source*) user_data)->user_data);
        return G_SOURCE_REMOVE;
        }

      inline void destroy_errors () noexcept
        {

          for (unsigned i = 0; i < __async_task_tuple_details<Tuple>::element_count; ++i)
            if (auto error = _errors [i]; NULL != error) _errors [i] = (g_error_free (error), nullptr);
        }

      inline void throw_errors ()
        {

          unsigned n_exceptions = 0;

          for (unsigned i = 0; i < __async_task_tuple_details<Tuple>::element_count; ++i)
            if (nullptr != _errors [i]) ++n_exceptions;

          if (0 < n_exceptions) if constexpr (auto ar = new std::exception_ptr [n_exceptions]; true)
            {

              for (unsigned i = 0, j = 0; i < __async_task_tuple_details<Tuple>::element_count; ++i)
                if (nullptr != _errors [i]) ar [j++] = glib_error::from_glib_error (g_steal_pointer (&_errors [i]));

              throw aggregate_error (ar, n_exceptions);
            }
        }
    };
}

template<asynclib::details::__async_task_tuple Tuple>
static inline auto operator co_await (Tuple&& tuple)
  noexcept (std::is_nothrow_constructible_v<asynclib::details::__async_task_tuple_awaitable<Tuple>, Tuple&&>)
{
  return asynclib::details::__async_task_tuple_awaitable<Tuple> (std::move (tuple));
}