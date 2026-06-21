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
#pragma once
#include <exception>
#include <glib.h>

G_BEGIN_DECLS

  GQuark asynclib_cpp_error_quark (void) G_GNUC_CONST;

G_END_DECLS

namespace asynclib
{

  std::exception_ptr from_glib_error (GError* error) noexcept;
  GError* to_glib_error (std::exception_ptr ptr) noexcept;

  class glib_error: public std::exception
    {

      struct _GError* _g_error = nullptr;
    public:

      inline glib_error () noexcept (std::is_nothrow_constructible_v<std::exception>):
                           glib_error (nullptr)
        { }

      inline glib_error (glib_error&& o) noexcept (std::is_nothrow_move_constructible_v<std::exception>):
                                         std::exception (std::move (o)), _g_error (o._g_error)
        { o._g_error = nullptr; }

      inline glib_error (const glib_error& o) noexcept (std::is_nothrow_copy_constructible_v<std::exception>):
                                             _g_error (g_error_copy (o._g_error))
        { }

      inline glib_error (GError* g_error) noexcept (std::is_nothrow_constructible_v<std::exception>):
                                          std::exception (), _g_error (g_error)
        { }

      inline ~glib_error () noexcept
        { ((NULL == _g_error) ? NULL : (_g_error = (g_error_free (_g_error), nullptr))); }

      static inline glib_error literal (GQuark domain, int code, const char* message)
          noexcept (std::is_nothrow_constructible_v<glib_error, GError*>)
        {
          return glib_error (g_error_new_literal (domain, code, message));
        }

      static inline glib_error printf (GQuark domain, int code, const char* format, ...)
          noexcept (std::is_nothrow_constructible_v<glib_error, GError*>) G_GNUC_PRINTF (3, 4)
        {

          va_list l;
          va_start (l, format);

          auto error = glib_error (g_error_new_valist (domain, code, format, l));
        return (va_end (l), error);
        }

      inline GError* steal () noexcept
        {
          auto g_error = _g_error;
          return (_g_error = nullptr, g_error);
        }

      virtual const char* what () const _GLIBCXX_TXN_SAFE_DYN noexcept override
        { return _g_error->message; }
    };
}