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
#include <asynclib/futureconcepts.h>
#include <asynclib/pollablehost.h>

namespace asynclib::details
{

  template<__future Future>
  struct __future_pollable_base: public __pollable_host_promise
    {

      virtual bool check () const noexcept override
        {
          return _timeout != _future.wait_for (_zero);
        }

    private:

      static constexpr auto _timeout = std::future_status::timeout;
      static constexpr auto _zero = std::chrono::milliseconds::zero ();

    protected:

      Future _future;

      inline __future_pollable_base (Future&& future) noexcept (std::is_nothrow_move_constructible_v<Future>):
                                                     _future (std::move (future))
        { }
    };
}