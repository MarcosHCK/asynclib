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
#include <asynclib/asynclib.h>
#include <tests/testing.h>
using namespace testing;

static asynclib::async_task<int> simple (int value) noexcept
{
co_return value;
}

static asynclib::async_task<int> wrapped (int value) noexcept
{

co_return (co_await simple (value) ^ 1);
}

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_ (TESTPATHROOT "/new/simple", []
    {

      auto rand = g_test_rand_int ();
      auto task = simple (rand);
      (void) task;
    });

  g_test_add_ (TESTPATHROOT "/new/wrapped", []
    {

      auto rand = g_test_rand_int ();
      auto task = wrapped (rand);
      (void) task;
    });

  g_test_add_ (TESTPATHROOT "/execute/simple", []
    {

      auto rand = g_test_rand_int ();
      auto task = simple (rand);

      struct D { decltype (task.end) end; guint ready; int result; }
        data = { .end = task.end, .ready = 0, .result = 0, };

      task.begin ([](GObject*, GAsyncResult* async_result, gpointer user_data)
        {
          auto p = (D*) user_data;
          auto e = (GError*) nullptr;

          p->result = p->end (async_result, &e);
          g_assert_no_error (e);
          g_atomic_int_set (&p->ready, 1);
        }, &data);

      for (auto main_context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&data.ready);)
        g_main_context_iteration (main_context, FALSE);

      g_assert_cmpint (0, ==, rand ^ data.result);
    });

  g_test_add_ (TESTPATHROOT "/execute/wrapped", []
    {

      auto rand = g_test_rand_int ();
      auto task = wrapped (rand);

      struct D { decltype (task.end) end; guint ready; int result; }
        data = { .end = task.end, .ready = 0, .result = 0, };

      task.begin ([](GObject*, GAsyncResult* async_result, gpointer user_data)
        {
          auto p = (D*) user_data;
          auto e = (GError*) nullptr;

          p->result = p->end (async_result, &e);
          g_assert_no_error (e);
          g_atomic_int_set (&p->ready, 1);
        }, &data);

      for (auto main_context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&data.ready);)
        g_main_context_iteration (main_context, FALSE);

      g_assert_cmpint (1, ==, rand ^ data.result);
    });

return g_test_run ();
}