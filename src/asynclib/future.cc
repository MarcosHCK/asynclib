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
#include <gio/gio.h>

class __future_host_impl: public asynclib::details::__future_host
{

  using __future_host_checker = asynclib::details::__future_host_checker;
  using __parent = asynclib::details::__future_host;
public:

  virtual void launch (__future_host_checker* checker) noexcept override;
};

__future_host_impl __p_future_host_impl;
asynclib::details::__future_host* asynclib::details::__future_host_impl = &__p_future_host_impl;

struct _TaskSource: public GSource
{

  asynclib::details::__future_host_checker* _checker;
};

static GSourceFuncs _TaskSource_funcs =
{

  .prepare = [](GSource* source, gint* out_timeout) noexcept -> gint
    { return (*out_timeout = 0, (int) FALSE); },

  .check = [](GSource* source) noexcept -> gint
    { return ((_TaskSource*) source)->_checker->check () ? TRUE : FALSE; },

  .dispatch = [](GSource* source, GSourceFunc callback, gpointer user_data) noexcept -> gboolean
    { return (((_TaskSource*) source)->_checker->dispatch (), G_SOURCE_REMOVE); },

  .finalize = [](GSource* source) noexcept -> void
    { return delete ((_TaskSource*) source)->_checker; },

  .closure_callback = NULL,
  .closure_marshal = NULL,
};

void __future_host_impl::launch (__future_host_checker* checker) noexcept
{

  auto main_context = g_main_context_get_thread_default ();
  auto task_source = g_source_new (&_TaskSource_funcs, sizeof (_TaskSource));

  ((_TaskSource*) task_source)->_checker = checker;

  g_source_set_priority (task_source, G_PRIORITY_HIGH);
  g_source_set_static_name (task_source, "[Asynclib::__future_host_impl::launch+task]");
  g_source_attach (task_source, main_context);

return g_source_unref (task_source);
}