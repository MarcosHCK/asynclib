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
#include <asynclib/gioexception.h>
#include <glib.h>

asynclib::gio_exception::~gio_exception () noexcept
{
  g_error_free ((GError*) _g_error);
}

const char* asynclib::gio_exception::what () const _GLIBCXX_TXN_SAFE_DYN noexcept
{
  return ((GError*) _g_error)->message;
}