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
#include <asynclib/aggregateerror.h>
#include <exception>
#include <sstream>
using namespace asynclib;

template<typename Range>
[[gnu::always_inline]]
static inline std::string make_message (Range&& range)
{

  std::stringstream ss;

  for (bool first = true; const std::exception_ptr& ptr: range)

    try
      { std::rethrow_exception (ptr); }
    catch (std::exception& exception)
      {  (! first ? ss << "\r\n" : (first = false, ss)) << exception.what (); }

return ss.str ();
}

aggregate_error::aggregate_error (std::exception_ptr* exceptions, unsigned n_exceptions):
  _exceptions (exceptions),
  _message (make_message (std::span (exceptions, n_exceptions))),
  _n_exceptions (n_exceptions)
{ }

[[gnu::always_inline]]
static inline std::exception_ptr* make_array (std::initializer_list<std::exception_ptr> list)
{

  auto exceptions = new std::exception_ptr [list.size ()];

  for (unsigned i = 0; const std::exception_ptr& ptr: list)
    exceptions [i++] = ptr;

return exceptions;
}

aggregate_error::aggregate_error (std::initializer_list<std::exception_ptr> list):
  aggregate_error (make_array (list), list.size ())
{ }