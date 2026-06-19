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
#define __GLIB_H_INSIDE__
#include <glib/gmacros.h>

struct _GError;
#define ASYNCLIB_CPP_ERROR (asynclib_cpp_error_quark ())

std::exception_ptr asynclib_cpp_error_get_ptr (struct _GError* error) noexcept;
struct _GError* asynclib_cpp_error_new (std::exception_ptr exception_ptr) noexcept;
unsigned asynclib_cpp_error_quark (void) G_GNUC_CONST;

namespace asynclib
{

  class glib_error: public std::exception
    {

      struct _GError* _g_error = nullptr;

      void unlink () noexcept;
    public:

      inline ~glib_error () noexcept
        { unlink (); }

      inline glib_error () noexcept (std::is_nothrow_constructible_v<std::exception>):
                           glib_error (nullptr)
        { }

      inline glib_error (glib_error&& o) noexcept (std::is_nothrow_move_constructible_v<std::exception>):
                                         std::exception (std::move (o)), _g_error (o._g_error)
        { o._g_error = nullptr; }

      glib_error (const glib_error&) noexcept (std::is_nothrow_copy_constructible_v<std::exception>);

      inline glib_error (struct _GError* g_error) noexcept (std::is_nothrow_constructible_v<std::exception>):
                                                  std::exception (), _g_error (g_error)
        { }

      constexpr const struct _GError* get_g_error () const noexcept { return _g_error; }

      static glib_error literal (unsigned domain, int code, const char* message)
        noexcept (std::is_nothrow_constructible_v<glib_error, struct _GError*>);

      static glib_error printf (unsigned domain, int code, const char* format, ...)
        noexcept (std::is_nothrow_constructible_v<glib_error, struct _GError*>) G_GNUC_PRINTF (3, 4);

      inline struct _GError* steal () noexcept
        { auto g_error = _g_error; return (_g_error = nullptr, g_error); }

      static void rethrow (struct _GError* error) G_GNUC_NORETURN;

      virtual const char* what () const _GLIBCXX_TXN_SAFE_DYN noexcept override;

      inline glib_error& operator= (glib_error&& o) noexcept
        {
          unlink ();
          return (std::swap (_g_error, o._g_error), *this);
        }

      glib_error& operator= (const glib_error& o) noexcept;

      inline bool operator== (std::nullptr_t) const noexcept
        { return nullptr == _g_error; }
    };
}