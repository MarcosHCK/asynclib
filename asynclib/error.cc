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

std::exception_ptr asynclib_cpp_error_get_ptr (struct _GError* error) noexcept
{

  auto priv = asynclib_cpp_error_get_private (error);
  auto ptr = std::exception_ptr (std::move (priv->exception_ptr));
return ptr;
}

GError* asynclib_cpp_error_new (std::exception_ptr exception_ptr) noexcept
{

  auto error = g_error_new_literal (ASYNCLIB_CPP_ERROR, 0, "c++ exception thrown");
  auto priv = asynclib_cpp_error_get_private (error);
  priv->exception_ptr = exception_ptr;
return error;
}

void glib_error::unlink () noexcept
{

  if (nullptr != _g_error)
    _g_error = (g_error_free (_g_error), nullptr);
}

glib_error::glib_error (const glib_error& o) noexcept (std::is_nothrow_copy_constructible_v<std::exception>):
                                             std::exception (o), _g_error (g_error_copy (o._g_error))
{ }

glib_error glib_error::literal (unsigned domain, int code, const char* message)
  noexcept (std::is_nothrow_constructible_v<glib_error, struct _GError*>)
{

  auto error = g_error_new_literal (domain, code, message);
return glib_error (error);
}

glib_error glib_error::printf (unsigned domain, int code, const char* format, ...)
  noexcept (std::is_nothrow_constructible_v<glib_error, struct _GError*>)
{

  va_list l;
  va_start (l, format);

  auto error = g_error_new_valist (domain, code, format, l);
  va_end (l);
return glib_error (error);
}

void glib_error::rethrow (struct _GError* error)
{

  if (ASYNCLIB_CPP_ERROR != error->domain)
    throw glib_error (error);

  auto exception_ptr = asynclib_cpp_error_get_ptr (error);
  g_error_free (error);

  std::rethrow_exception (exception_ptr);
};

const char* glib_error::what () const _GLIBCXX_TXN_SAFE_DYN noexcept
{
  return ((GError*) _g_error)->message;
}

glib_error& glib_error::operator= (const glib_error& o) noexcept
{

  if (nullptr != _g_error)
    _g_error = (g_error_free (_g_error), nullptr);

return (_g_error = g_error_copy (o._g_error), *this);
}
