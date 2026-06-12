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
#include <asynclib/impl/checkthread.h>
#include <asynclib/future.h>
#include <asynclib/future_co.h>
#include <asynclib/future_op.h>

class __future_host_impl: public asynclib::details::__future_host
{

  using __future_host_checker = asynclib::details::__future_host_checker;
  using __parent = asynclib::details::__future_host;
  asynclib::details::__check_thread _check_thread;
public:

  inline __future_host_impl () noexcept:
    _check_thread (asynclib::details::__check_thread ()) { }

  virtual void launch (__future_host_checker* checker) noexcept override;
};

__future_host_impl __p_future_host_impl;
asynclib::details::__future_host* asynclib::details::__future_host_impl = &__p_future_host_impl;

void __future_host_impl::launch (__future_host_checker* checker) noexcept
{

  GMainContext* main_context;

  _check_thread.push (main_context = g_main_context_ref_thread_default (), checker);
  g_main_context_unref (main_context);
}