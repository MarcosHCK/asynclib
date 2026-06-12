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
#include <config.h>
#include <asynclib/future.h>
#include <asynclib/future_co.h>
#include <asynclib/future_op.h>
#include <asynclib/invoker.h>
#include <asynclib/slice.h>
#include <gio/gio.h>

class __future_host_impl: public asynclib::details::__future_host
{

  using __future_host_checker = asynclib::details::__future_host_checker;
  using __parent = asynclib::details::__future_host;
  class _checker_info;

  GThreadPool* _thread_pool;
public:

  inline __future_host_impl () noexcept;
  inline ~__future_host_impl () noexcept;

  static void check (_checker_info* info, __future_host_impl* thread_pool) noexcept;
  virtual void launch (__future_host_checker* checker) noexcept override;
};

__future_host_impl __p_future_host_impl;
asynclib::details::__future_host* asynclib::details::__future_host_impl = &__p_future_host_impl;

class __future_host_impl::_checker_info
{

  __future_host_checker* _checker;
  GMainContext* _main_context;

  static void invoke (gpointer _p_checker) noexcept
    {
      auto checker = (__future_host_checker*) _p_checker;
    return checker->dispatch ();
    }

  static void notify (gpointer _p_checker) noexcept
    {
      auto checker = (__future_host_checker*) _p_checker;
    return delete checker;
    }

public:

  inline ~_checker_info () noexcept
    {

      _checker = (NULL == _checker) ? nullptr : (delete _checker, nullptr);
      g_main_context_unref (_main_context);
    }

  inline _checker_info (__future_host_checker* checker) noexcept:
      _checker (checker),
      _main_context (g_main_context_ref_thread_default ())
    { }

  inline bool check () noexcept
    { return _checker->check (); }

  inline void dispatch () noexcept
    {
      auto checker = g_steal_pointer (&_checker);
    return asynclib_invoke_in_context (_main_context, invoke, checker, notify);
    }
};

__future_host_impl::__future_host_impl () noexcept
{

  const auto func = (GFunc) check;
  const auto notify = (GDestroyNotify) g_slice_free_<_checker_info>;

  GError* tmperr = NULL;

  if (_thread_pool = g_thread_pool_new_full (func, this, notify, -1, FALSE, &tmperr); G_UNLIKELY (NULL != tmperr))
    {

      const guint code = tmperr->code;
      const gchar* domain = g_quark_to_string (tmperr->domain);
      const gchar* message = tmperr->message;

      g_error ("g_thread_pool_new_full ()!: %s: %u: %s", domain, code, message);

      g_error_free (tmperr);
      g_assert_not_reached ();
    }
}

__future_host_impl::~__future_host_impl () noexcept
{

  g_thread_pool_free (_thread_pool, FALSE, TRUE);
}

void __future_host_impl::check (_checker_info* info, __future_host_impl* impl) noexcept
{

  if (true == info->check ())

    { info->dispatch ();
      g_slice_free_<_checker_info> (info); }
  else
    { g_thread_pool_push (impl->_thread_pool, info, NULL); }
}

void __future_host_impl::launch (__future_host_checker* checker) noexcept
{

  g_thread_pool_push (_thread_pool, g_slice_new_<_checker_info> (checker), NULL);
}