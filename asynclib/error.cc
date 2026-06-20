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
#include <asynclib/error.h>
#include <exception>
#include <glib.h>
using namespace asynclib;

struct AsynclibCppErrorPrivate
{
  std::exception_ptr exception_ptr;
};

#define ASYNCLIB_CPP_ERROR (asynclib_cpp_error_quark ())

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

static std::exception_ptr asynclib_cpp_error_get_ptr (struct _GError* error) noexcept
{

  auto priv = asynclib_cpp_error_get_private (error);
  auto ptr = std::exception_ptr (std::move (priv->exception_ptr));
return ptr;
}

static GError* asynclib_cpp_error_new (std::exception_ptr exception_ptr) noexcept
{

  auto error = g_error_new_literal (ASYNCLIB_CPP_ERROR, 0, "c++ exception thrown");
  auto priv = asynclib_cpp_error_get_private (error);
  priv->exception_ptr = exception_ptr;
return error;
}

std::exception_ptr asynclib::from_glib_error (GError* error) noexcept
{

  if (std::exception_ptr ptr; ASYNCLIB_CPP_ERROR != error->domain)

    return std::make_exception_ptr (glib_error (error));
  else
    return (ptr = asynclib_cpp_error_get_ptr (error), g_error_free (error), ptr);
}

GError* asynclib::to_glib_error (std::exception_ptr ptr) noexcept
{

  try
    { std::rethrow_exception (ptr); }
  catch (asynclib::glib_error& exception)
    { return exception.steal (); }
  catch (...)
    { return asynclib_cpp_error_new (std::current_exception ()); }
}