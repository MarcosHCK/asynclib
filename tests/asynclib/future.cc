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

static std::future<int> xor1 (int value)
{
co_return value ^ 1;
}

static std::future<int> xor2 (int value)
{
  auto t = co_await xor1 (value);
co_return t ^ 2;
}

static std::future<int> xor2r (int value)
{
  auto f = xor1 (value);
co_return (co_await f) ^ 2;
}

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_ (TESTPATHROOT "/chain/action", []
    {

      auto n = (guint) g_test_rand_int ();
      auto r = n;

      guint ready = 0;

      std::async (std::launch::async, [n] { return n; })
        >> [&](std::future<guint>& future) noexcept { r = future.get (); g_atomic_int_set (&ready, 1); };

      for (auto context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&ready); )
        g_main_context_iteration (context, FALSE);

      g_assert_cmpuint (0, ==, n ^ r);
    });

  g_test_add_ (TESTPATHROOT "/chain/transform", []
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

  g_test_add_ (TESTPATHROOT "/coroutine/simple", []
    {

      auto n = (int) g_test_rand_int ();
      auto r = n;

      guint ready = 0;

      xor2 (n)
        >> [&](std::future<int>& future) noexcept { r = future.get (); g_atomic_int_set (&ready, 1); };;

      for (auto context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&ready); )
        g_main_context_iteration (context, FALSE);

      g_assert_cmpuint (3, ==, n ^ r);
    });

  g_test_add_ (TESTPATHROOT "/coroutine/lvalue", []
    {

      auto n = (int) g_test_rand_int ();
      auto r = n;

      guint ready = 0;

      xor2r (n)
        >> [&](std::future<int>& future) noexcept { r = future.get (); g_atomic_int_set (&ready, 1); };;

      for (auto context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&ready); )
        g_main_context_iteration (context, FALSE);

      g_assert_cmpuint (3, ==, n ^ r);
    });

return g_test_run ();
}