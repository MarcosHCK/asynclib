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
#include <asynclib/futureasyncinvocation.h>
#include <tests/testing.h>
using namespace testing;

static asynclib::async_task<guint> simple (guint value) noexcept
{
co_return value;
}

static std::future<guint> wrapped (guint value) noexcept
{

  auto task = asynclib::to_future (simple (value));
co_return (co_await task ^ 1);
}

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_ (TESTPATHROOT "/simple", []
    {

      guint ready = 0;
      guint r, n = g_test_rand_int ();

      asynclib::to_future (simple (n))
        >> [&](std::future<guint>& future) noexcept { r = future.get (); g_atomic_int_set (&ready, 1); };

      for (auto context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&ready); )
        g_main_context_iteration (context, FALSE);

      g_assert_cmpuint (0, ==, r ^ n);
    });

  g_test_add_ (TESTPATHROOT "/wrapped", []
    {

      guint ready = 0;
      guint r, n = g_test_rand_int ();

      wrapped (n)
        >> [&](std::future<guint>& future) noexcept { r = future.get (); g_atomic_int_set (&ready, 1); };

      for (auto context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&ready); )
        g_main_context_iteration (context, FALSE);

      g_assert_cmpuint (1, ==, r ^ n);
    });

return g_test_run ();
}