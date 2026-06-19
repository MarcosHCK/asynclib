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
#include <future>
#include <type_traits>

namespace asynclib::details
{

  struct __future_unbound_tag;

  template<typename Future,
           typename Value = __future_unbound_tag>
  concept __future = requires (Future f, std::chrono::milliseconds d)
    {

      { f.get () };
      { f.wait_for (d) } -> std::same_as<std::future_status>;
      requires std::movable<Future>;

      requires std::same_as<Value, __future_unbound_tag> || requires ()
        {
          { f.get () } -> std::same_as<Value>;
        };
    };

  template<typename Promise,
           typename Value>
  concept __promise = requires (Promise p, std::exception_ptr e)
    {

      { p.get_future () } -> __future;
      { p.set_exception (e) } -> std::same_as<void>;
      requires std::movable<Promise>;

      requires (std::is_same_v<Value, void> && requires ()
        {
          { p.set_value () } -> std::same_as<void>;
        }) || requires (Value v)
        {
          { p.set_value (v) } -> std::same_as<void>;
        };

      requires std::is_constructible_v<Promise>;
    };

  template<typename Future,
           typename Value = __future_unbound_tag>
  concept __shared_future = __future<Future, Value> && std::copyable<Future>;

  template<typename Future,
           typename Value = __future_unbound_tag>
  concept __shareable_future = __future<Future, Value> && requires (Future f)
    {
      { f.share () } -> __shared_future;
    };

  template<typename Future,
           typename Value = __future_unbound_tag>
  concept __shareable_future_nothrow = __shareable_future<Future, Value> && requires (Future f)
    {
      requires std::is_nothrow_invocable_v<decltype (f.share)>;
    };
}