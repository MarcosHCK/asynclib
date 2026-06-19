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
#include <asynclib/pollablehost.h>
#include <glib.h>
using namespace asynclib::details;

class __pollable_host_impl: public __pollable_host
{
public:
  virtual void launch (__pollable_host_promise* promise) noexcept override;
};

__pollable_host_impl __p_pollable_host_impl;
__pollable_host* asynclib::details::__p_pollable_host = &__p_pollable_host_impl;

struct __task_source
{

  GSource parent;
  __pollable_host_promise* promise;

  static inline GSource* create (__pollable_host_promise* promise) noexcept;
};

static GSourceFuncs source_funcs =
{

  .prepare = [](GSource* source, gint* out_timeout) noexcept -> gboolean
    {
      *out_timeout = 0;
      return FALSE;
    },

  .check = [](GSource* source) noexcept -> gboolean
    { return ((__task_source*) source)->promise->check () ? TRUE : FALSE; },

  .dispatch = [](GSource* source, GSourceFunc callback, gpointer user_data) noexcept -> gboolean
    {

      auto checker = ((__task_source*) source)->promise;
    return (checker->dispatch (), G_SOURCE_REMOVE);
    },

  .finalize = [](GSource* source) noexcept -> void
    {

      auto checker = ((__task_source*) source)->promise;
    return __pollable_host_promise::destroy (checker);
    },

  .closure_callback = NULL,
  .closure_marshal = NULL,
};

inline GSource* __task_source::create (__pollable_host_promise* promise) noexcept
{

  auto source = g_source_new (&source_funcs, sizeof (__task_source));
  ((__task_source*) source)->promise = promise;
return source;
}

void __pollable_host_impl::launch (__pollable_host_promise* promise) noexcept
{

  auto source = __task_source::create (promise);

  g_source_set_priority (source, G_PRIORITY_LOW);
  g_source_set_static_name (source, "[asynclib::details::__pollable_host::__task_source]");

  g_source_attach (source, g_main_context_get_thread_default ());
  g_source_unref (source);
}

void* __pollable_host_promise::alloc (size_t size, size_t alignment) noexcept
{
return g_malloc (size);
}

void __pollable_host_promise::free (void* ptr) noexcept
{
  g_free (ptr);
}