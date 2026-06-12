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
#include <asynclib/impl/checksource.h>
#include <asynclib/impl/invoker.h>
using __future_host_checker = asynclib::details::__future_host_checker;

struct __task_source
{

  GSource parent;
  asynclib::details::__future_host_checker* checker;
  GMainContext* target_context;
};

static void dispatch (gpointer _p_checker) noexcept
{
  ((__future_host_checker*) _p_checker)->dispatch ();
}

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

      auto checker = g_steal_pointer (&((__task_source*) source)->checker);
      _asynclib_invoke_in_context (((__task_source*) source)->target_context, dispatch, checker, __future_host_checker::destroy);
      return G_SOURCE_REMOVE;
    },

  .finalize = [](GSource* source) noexcept -> void
    {

      if (auto checker = ((__task_source*) source)->checker; G_UNLIKELY (NULL != checker))
        __future_host_checker::destroy (checker);

      g_main_context_unref (((__task_source*) source)->target_context);
    },

  .closure_callback = NULL,
  .closure_marshal = NULL,
};

GSource* _asynclib_check_source_new (GMainContext* target_context, asynclib::details::__future_host_checker* checker) noexcept
{

  auto source = g_source_new (&source_funcs, sizeof (__task_source));

  ((__task_source*) source)->checker = checker;
  ((__task_source*) source)->target_context = g_main_context_ref (target_context);

  g_source_set_priority (source, G_PRIORITY_HIGH);
  g_source_set_static_name (source, "[Asynclib::CheckSource]");
return source;
}