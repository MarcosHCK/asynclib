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
#include <initializer_list>
#include <span>
#include <string>

namespace asynclib
{

  class aggregate_error: public std::exception
    {

      std::exception_ptr* _exceptions;
      std::string _message;
      unsigned _n_exceptions;
    public:

      inline aggregate_error (): aggregate_error ({ })
        { }

      aggregate_error (std::exception_ptr* exceptions, unsigned n_exceptions);
      aggregate_error (std::initializer_list<std::exception_ptr> list);

      inline constexpr std::span<const std::exception_ptr> get_exceptions () const noexcept
        { return std::span<const std::exception_ptr> (_exceptions, _n_exceptions); }

      virtual const char* what () const _GLIBCXX_TXN_SAFE_DYN noexcept override
        { return _message.c_str (); }
    };
}