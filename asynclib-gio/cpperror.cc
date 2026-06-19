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
#include <asynclib-gio/cpperror.h>

struct AsynclibCppErrorPrivate
{
  std::exception_ptr exception_ptr;
};

static void asynclib_cpp_error_private_clear (AsynclibCppErrorPrivate* priv) noexcept;
static void asynclib_cpp_error_private_copy (const AsynclibCppErrorPrivate* src, AsynclibCppErrorPrivate* dst) noexcept;
static void asynclib_cpp_error_private_init (AsynclibCppErrorPrivate* priv) noexcept;

G_DEFINE_EXTENDED_ERROR (AsynclibCppError, asynclib_cpp_error)

static void asynclib_cpp_error_private_clear (AsynclibCppErrorPrivate* priv) noexcept
{
  (&priv->exception_ptr)->~exception_ptr ();
}

static void asynclib_cpp_error_private_copy (const AsynclibCppErrorPrivate* src, AsynclibCppErrorPrivate* dst) noexcept
{
  new (&dst->exception_ptr) std::exception_ptr (src->exception_ptr);
}

static void asynclib_cpp_error_private_init (AsynclibCppErrorPrivate* dst) noexcept
{
  new (&dst->exception_ptr) std::exception_ptr ();
}

GError* asynclib_cpp_error_new (std::exception_ptr exception_ptr)
{

  auto error = g_error_new_literal (ASYNCLIB_CPP_ERROR, 0, "c++ exception thrown");
  auto priv = asynclib_cpp_error_get_private (error);
  priv->exception_ptr = exception_ptr;

return error;
}