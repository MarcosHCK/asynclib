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
#include <chrono>
#include <future>

namespace asynclib
{

  namespace details
    {

      struct __future_unbound_tag;

      template<typename _Future,
               typename _Value = __future_unbound_tag>
      concept __future = requires (_Future f, std::chrono::milliseconds d)
        {

          { f.get () };
          { f.wait_for (d) } -> std::same_as<std::future_status>;
          requires std::movable<_Future>;

          requires std::same_as<_Value, __future_unbound_tag> || requires ()
            {
              { f.get () } -> std::same_as<_Value>;
            };
        };

      template<typename _Promise,
               typename _Value = void>
      concept __promise = requires (_Promise p, std::exception_ptr e)
        {

          { p.get_future () } -> __future;
          { p.set_exception (e) } -> std::same_as<void>;
          requires std::movable<_Promise>;

          requires (std::is_same_v<_Value, void> && requires ()
            {
              { p.set_value () } -> std::same_as<void>;
            }) || requires (_Value v)
            {
              { p.set_value (v) } -> std::same_as<void>;
            };
        };

      template<typename _Future,
               typename _Value = __future_unbound_tag>
      concept __shared_future = __future<_Future, _Value> && std::copyable<_Future>;

      template<typename _Future,
               typename _Value = __future_unbound_tag>
      concept __shareable_future = __future<_Future, _Value> && requires (_Future f)
        {
          { f.share () } -> __shared_future;
        };

      class __future_host;
      struct __future_host_checker;
      template<__future _Future> struct __future_link_base;

      extern __future_host* __future_host_impl;
    };

  class details::__future_host
    {

      typedef void *mptr;
    public:

      virtual ~__future_host () noexcept { }
      virtual void launch (__future_host_checker* checker) noexcept = 0;
    };

  struct details::__future_host_checker
    {

      virtual ~__future_host_checker () noexcept { }

      virtual bool check () const noexcept = 0;
      virtual void dispatch () noexcept = 0;

      template<std::derived_from<__future_host_checker> _Class,
               typename... Args>
      static inline _Class* create (Args&&... args) noexcept
        {

          auto checker = new _Class (std::forward<Args> (args) ...);
        return (__future_host_impl->launch (checker), checker);
        }

      static void destroy (void* _p_checker) noexcept
        { delete (__future_host_checker*) _p_checker; }
    };

  template<details::__future _Future> struct details::__future_link_base: public __future_host_checker
    {

      virtual bool check () const noexcept override
        {
          return _timeout != _future.wait_for (_zero);
        }

    private:

      static constexpr auto _timeout = std::future_status::timeout;
      static constexpr auto _zero = std::chrono::milliseconds::zero ();

    protected:

      _Future _future;

      inline __future_link_base (_Future&& future) noexcept (std::is_nothrow_move_constructible_v<_Future>):
                                                  _future (std::move (future))
        { }
    };
}