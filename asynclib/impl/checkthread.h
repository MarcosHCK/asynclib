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
#include <asynclib/future.h>
#include <glib.h>

namespace asynclib::details
{

  struct __check_thread
    {

      GMainContext* _main_context;
      GMainLoop* _main_loop;
      GThread* _thread;

      void entry () noexcept;

    public:

      ~__check_thread () noexcept;
      __check_thread () noexcept;

      void push (GMainContext* main_context, __future_host_checker* checker) noexcept;
    };
}