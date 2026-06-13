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
#include <new>
#include <tests/testing.h>

static void destroy_func (gpointer mptr)
{

  ((std::move_only_function<void ()>*) mptr)->~move_only_function ();
  g_slice_free1 (sizeof (std::move_only_function<void ()>), mptr);
}

static void test_func (gconstpointer mptr)
{

  (* ((std::move_only_function<void ()>*) mptr)) ();
}

void testing::g_test_add_function (const char* path, std::move_only_function<void ()>&& func)
{

  auto mptr = g_slice_alloc0 (sizeof (std::move_only_function<void ()>));
  auto data = new (mptr) std::move_only_function<void ()> (std::move (func));

  g_test_add_data_func_full (path, data, test_func, destroy_func);
}