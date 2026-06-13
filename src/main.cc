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

static void delay_worker (GTask* task, void* source_object, void* task_data, GCancellable* cancellable)
{

  g_usleep ((gulong) (((double) G_USEC_PER_SEC / (double) 1000) * (double) GPOINTER_TO_UINT (task_data)));
  g_task_return_boolean (task, TRUE);
}

static inline void delay_async (guint interval, GAsyncReadyCallback callback, gpointer user_data)
{

  auto task = g_task_new (NULL, NULL, callback, user_data);

  g_task_set_task_data (task, GUINT_TO_POINTER (interval), NULL);
  g_task_run_in_thread (task, delay_worker);
}

static inline void delay_finish (GAsyncResult* result, GError** user_data)
{

  g_task_propagate_boolean ((GTask*) result, user_data);
}

asynclib::gio_task<delay_async, delay_finish> delay_task;

template<typename T>
static inline std::future<int> delayed_native (T&& _value, guint interval) noexcept
{

  T value = std::forward<T> (_value);

co_return (co_await delay (interval), value);
}

template<typename T>
static inline std::future<int> delayed_task (T&& _value, guint interval) noexcept
{

  T value = std::forward<T> (_value);

co_return (co_await delay_task (interval), value);
}

int main (int argc, char* argv[])
{

  GMainContext* main_context = g_main_context_ref_thread_default ();
  GMainLoop* main_loop = g_main_loop_new (main_context, FALSE);

  int value = g_random_int ();

  g_print ("generated (value = %i)\n", value);

  delayed_native (value, 2000) >> [](std::future<int>& future) noexcept -> void
    { g_print ("delayed_native future fulfilled (value = %i)\n", future.get ()); };

  delayed_task (value, 2000) >> [](std::future<int>& future) noexcept -> void
    { g_print ("delayed_task future fulfilled (value = %i)\n", future.get ()); };

  g_print ("running loop\n");
  g_main_loop_run (main_loop);

  g_main_context_unref (main_context);
  g_main_loop_unref (main_loop);
return 0;
}