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
#include <cstddef>
#include <utility>

namespace asynclib::details
{

  class __pollable_host;
  class __pollable_host_promise;

  class __pollable_host
    {

      typedef void *mptr;
    public:

      virtual ~__pollable_host () noexcept { }
      virtual void launch (__pollable_host_promise* checker) noexcept = 0;
    };

  extern __pollable_host* __p_pollable_host;

  class __pollable_host_promise
    {
    public:

      virtual ~__pollable_host_promise () noexcept { }

      virtual bool check () const noexcept = 0;
      virtual void dispatch () noexcept = 0;

      static void* alloc (size_t size, size_t alignment) noexcept;
      static void free (void* ptr) noexcept;

      template<std::derived_from<__pollable_host_promise> T,
               typename... Args>
      static inline __pollable_host_promise* create (Args&&... args) noexcept (std::is_nothrow_constructible_v<T, Args ...>)
        {

          auto ptr = alloc (sizeof (T), alignof (T));
          auto val = new (ptr) T (std::forward<Args> (args) ...);
        return (__pollable_host_promise*) val;
        }

      static void destroy (void* ptr) noexcept
        {

          ((__pollable_host_promise*) ptr)->~__pollable_host_promise ();
          free (ptr);
        }

      template<std::derived_from<__pollable_host_promise> T,
               typename... Args>
      static inline void launch (Args&&... args) noexcept (std::is_nothrow_constructible_v<T, Args ...>)
        {

          auto promise = create<T> (std::forward<Args> (args) ...);
          __p_pollable_host->launch (promise);
        }
    };
}