/* Copyright (C) 2025-2026 MarcosHCK
 * This file is part of futures.
 *
 * futures is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * futures is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <asynclib/asynclib.h>
#include <glib.h>

static inline std::future<void> delay (guint interval) noexcept
{

  auto promise = new std::promise<void> ();

  g_timeout_add_full (G_PRIORITY_DEFAULT, interval,
    [](gpointer p) -> gboolean { (*(decltype (promise)) p).set_value ();
                                 return G_SOURCE_REMOVE; },
    promise,
    [](gpointer p) -> void { return delete (decltype (promise)) p; });

return promise->get_future ();
}

template<typename T>
static inline std::future<int> delayed (T&& _value, guint interval) noexcept
{

  T value = std::forward<T> (_value);

co_return (co_await delay (interval), value);
}

int main (int argc, char* argv[])
{

  GMainContext* main_context = g_main_context_ref_thread_default ();
  GMainLoop* main_loop = g_main_loop_new (main_context, FALSE);

  int value = g_random_int ();

  g_print ("generated (value = %i)\n", value);

  delayed (value, 2000) >> [](std::future<int>& future) noexcept -> void
    { g_print ("future fulfilled (value = %i)\n", future.get ()); };

  g_print ("running loop\n");
  g_main_loop_run (main_loop);

  g_main_context_unref (main_context);
  g_main_loop_unref (main_loop);
return 0;
}