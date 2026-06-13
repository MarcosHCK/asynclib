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
#include <asynclib/gioerror.h>
#include <glib.h>
using namespace asynclib;

gio_error::~gio_error () noexcept
{

  if (nullptr != _g_error)
    _g_error = (g_error_free ((GError*) _g_error), nullptr);
}

gio_error::gio_error (const gio_error& o) noexcept (std::is_nothrow_copy_constructible_v<_parent>):
                                         _parent (o), _g_error (g_error_copy ((GError*) o._g_error))
{ }

gio_error::gio_error (unsigned domain, int code, const char* message) noexcept (std::is_nothrow_constructible_v<_parent>):
                                                                     _parent (), _g_error (g_error_new_literal (domain, code, message))
{ }

gio_error::gio_error (unsigned domain, int code, const char* format, ...) noexcept (std::is_nothrow_constructible_v<_parent>):
                                                                         _parent ()
{

  va_list l;
  va_start (l, format);

  _g_error = g_error_new_valist (domain, code, format, l);
  va_end (l);
}

const char* gio_error::what () const _GLIBCXX_TXN_SAFE_DYN noexcept
{
  return ((GError*) _g_error)->message;
}