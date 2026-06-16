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
#include <glib.h>

class __future_host_impl: public asynclib::details::__future_host
{

  using __future_host_checker = asynclib::details::__future_host_checker;
  using __parent = asynclib::details::__future_host;
public:
  virtual void launch (__future_host_checker* checker) noexcept override;
};

__future_host_impl __p_future_host_impl;
asynclib::details::__future_host* asynclib::details::__future_host_impl = &__p_future_host_impl;

struct __task_source
{

  GSource parent;
  asynclib::details::__future_host_checker* checker;
};

using __future_host_checker = asynclib::details::__future_host_checker;

static GSourceFuncs source_funcs =
{

  .prepare = [](GSource* source, gint* out_timeout) noexcept -> gboolean
    {
      *out_timeout = 0;
      return FALSE;
    },

  .check = [](GSource* source) noexcept -> gboolean
    { return ((__task_source*) source)->checker->check () ? TRUE : FALSE; },

  .dispatch = [](GSource* source, GSourceFunc callback, gpointer user_data) noexcept -> gboolean
    {

      auto checker = ((__task_source*) source)->checker;
    return (checker->dispatch (), G_SOURCE_REMOVE);
    },

  .finalize = [](GSource* source) noexcept -> void
    {

      auto checker = ((__task_source*) source)->checker;
    return __future_host_checker::destroy (checker);
    },

  .closure_callback = NULL,
  .closure_marshal = NULL,
};

void __future_host_impl::launch (__future_host_checker* checker) noexcept
{

  auto source = g_source_new (&source_funcs, sizeof (__task_source));

  g_source_set_priority (source, G_PRIORITY_LOW);
  g_source_set_static_name (source, "[asynclib::__task_source]");
  ((__task_source*) source)->checker = checker;

  g_source_attach (source, g_main_context_get_thread_default ());
  g_source_unref (source);
}