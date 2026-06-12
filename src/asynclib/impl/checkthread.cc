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
#include <asynclib/impl/checkthread.h>
using namespace asynclib::details;
using __future_host_checker = asynclib::details::__future_host_checker;

void __check_thread::entry () noexcept
{
  
  auto parent_context = _main_context;

  auto main_context = g_main_context_new ();
  auto main_loop = g_main_loop_new (main_context, FALSE);

  _main_context = g_main_context_ref (main_context);
  _main_loop = g_main_loop_ref (main_loop);
  g_main_context_wakeup (parent_context);

  g_main_context_push_thread_default (main_context);
  g_main_loop_run (main_loop);

  g_main_context_pop_thread_default (main_context);
  g_main_context_unref (main_context);

return g_main_loop_unref (main_loop);
}

__check_thread::~__check_thread () noexcept
{

  g_main_context_unref (_main_context);

  g_main_loop_quit (_main_loop);
  g_main_loop_unref (_main_loop);

  g_thread_join (_thread);
  g_thread_unref (_thread);
}

__check_thread::__check_thread () noexcept
{

  _main_context = g_main_context_get_thread_default ();

  _thread = g_thread_new ("[Asynclib::CheckThread]",
    [](gpointer _p_this) noexcept -> gpointer
    { return (((__check_thread*) _p_this)->entry (), nullptr); }, this);

  for (auto main_context = g_main_context_get_thread_default (); NULL == _main_loop;)
    g_main_context_iteration (main_context, FALSE);
}

void __check_thread::push (GMainContext* main_context, __future_host_checker* checker) noexcept
{

  GSource* source;

  g_source_attach (source = _asynclib_check_source_new (main_context, checker), _main_context);
  g_source_unref (source);
}