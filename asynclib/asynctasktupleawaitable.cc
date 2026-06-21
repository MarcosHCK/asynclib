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
#include <asynclib/asynctasktupleawaitable.h>
#include <glib.h>
using namespace asynclib::details;

#define _g_object_unref0(var) ((NULL == var) ? NULL : (var = (g_object_unref (var), nullptr)))

static GSourceFuncs _invoke_source_funcs =
{

  .prepare = [](GSource* source, gint* timeout) noexcept -> gboolean
    {
      return (*timeout = 0, TRUE);
    },

  .check = [](GSource* source) noexcept -> gboolean
    {
      return TRUE;
    },

  .dispatch = [](GSource* source, GSourceFunc func, gpointer data) noexcept -> gboolean
    {
      return func (data);
    },

  .finalize = [](GSource* source) noexcept -> void
    {
      _g_object_unref0 (((__async_task_tuple_awaitable_base_type_erased::_invoke_source*) source)->async_result);
      _g_object_unref0 (((__async_task_tuple_awaitable_base_type_erased::_invoke_source*) source)->source_object);
    },

  .closure_callback = nullptr,
  .closure_marshal = nullptr,
};

GSource* __async_task_tuple_awaitable_base_type_erased::invoke_source_new (GObject* source_object, GAsyncResult* async_result, gpointer user_data) noexcept
{

  auto source = g_source_new (&_invoke_source_funcs, sizeof (_invoke_source));

  ((_invoke_source*) source)->async_result = g_object_ref (async_result);
  ((_invoke_source*) source)->user_data = user_data;
  ((_invoke_source*) source)->source_object = NULL == source_object ? NULL : g_object_ref (source_object);

  g_source_set_priority (source, G_PRIORITY_HIGH);
  g_source_set_static_name (source, "asynclib::details::__async_task_tuple_awaitable_base_type_erased::_invoke_source");
return source;
}