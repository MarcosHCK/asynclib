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
using namespace std::chrono_literals;
using namespace testing;

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_ (TESTPATHROOT "/notify", []
    {

      auto n = (guint) g_test_rand_int ();
      auto r = n;

      guint ready = 0;

      std::async (std::launch::async, [n] { return n ^ 1; })
        >> [&](std::future<guint>& future) noexcept { r = future.get (); g_atomic_int_set (&ready, 1); };

      for (auto context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&ready); )
        g_main_context_iteration (context, FALSE);

      g_assert_cmpuint (1, ==, n ^ r);
    });

  g_test_add_ (TESTPATHROOT "/transform", []
    {

      auto n = (guint) g_test_rand_int ();
      auto r = n;

      guint ready = 0;

      (std::async (std::launch::async, [n] { return n; })
        | [](std::future<guint>& future) { return future.get () ^ 1; })
       >> [&](std::future<guint>& future) noexcept { r = future.get (); g_atomic_int_set (&ready, 1); };

      for (auto context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&ready); )
        g_main_context_iteration (context, FALSE);

      g_assert_cmpuint (1, ==, n ^ r);
    });

return g_test_run ();
}