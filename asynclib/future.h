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
#include <functional>
#include <future>

namespace asynclib
{

  template<typename T>
    struct unwrap_future { };

  template<typename T>
    struct unwrap_future<T&> { using type = unwrap_future<T>::type; };

  template<typename T>
    struct unwrap_future<T&&> { using type = unwrap_future<T>::type; };

  template<typename T>
    struct unwrap_future<std::future<T>> { using type = T; };

  template<typename T>
    struct unwrap_future<std::shared_future<T>> { using type = T; };

  template<typename T>
    using unwrap_future_t = unwrap_future<T>::type;

  namespace details
    {

      template<typename _Future>
      concept Future = std::is_base_of_v<std::__future_base, std::remove_reference_t<_Future>>;

      class __future_host;
      struct __future_host_checker;
      template<Future _Future> struct __future_link_base;
      template<Future _Future> struct __future_link_action;
      template<Future _Future, typename R> struct __future_link_result;

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

      static void destroy (void* _p_checker) noexcept
        { delete (__future_host_checker*) _p_checker; }
    };

  template<details::Future _Future> struct details::__future_link_base: public __future_host_checker
    {

      typedef _Future future_type;
      typedef unwrap_future_t<_Future> value_type;
      static_assert (std::is_base_of_v<std::__future_base, std::remove_reference_t<_Future>>);

      future_type _future;

      virtual bool check () const noexcept override
        { return _timeout != _future.wait_for (_zero); }

    protected:

      inline __future_link_base (future_type&& future) noexcept: _future (std::forward<_Future> (future))
        { }

    private:

      static constexpr auto _timeout = std::future_status::timeout;
      static constexpr auto _zero = std::chrono::milliseconds::zero ();
    };

  template<details::Future _Future> struct details::__future_link_action: public __future_link_base<_Future>
    {

      typedef __future_link_base<_Future> _parent;
      typedef __future_link_base<_Future>::future_type future_type;
      typedef std::move_only_function<void (future_type&)> action_type;

      static inline void create (action_type&& action, future_type&& future) noexcept
        {

          auto impl = new __future_link_action (std::move (action), std::forward<_Future> (future));
          __future_host_impl->launch (impl);
        }

      virtual void dispatch () noexcept override
        {
          return _action (this->_future);
        }

    private:

      action_type _action;

      inline __future_link_action (action_type&& action, future_type&& future)
      noexcept:
             _parent (std::forward<_Future> (future)), _action (std::move (action))
        { }
    };

  template<details::Future _Future, typename R> struct details::__future_link_result: public __future_link_base<_Future>
    {

      typedef __future_link_base<_Future> _parent;
      typedef __future_link_base<_Future>::future_type future_type;
      typedef std::move_only_function<R (future_type&)> action_type;
      typedef std::promise<R> promise_type;

      static inline std::future<R> create (action_type&& action, future_type&& future)
        {

          auto promise = std::promise<R> ();
          auto next = promise.get_future ();

          auto impl = new __future_link_result (std::move (action), std::forward<_Future> (future), std::move (promise));
          __future_host_impl->launch (impl);
        return next;
        }

      virtual void dispatch () noexcept override
        {
          try
            { _promise.set_value (_action (this->_future)); }
          catch (...)
            { _promise.set_exception (std::current_exception ()); }
        }

    private:

      action_type _action;
      promise_type _promise;

      inline __future_link_result (action_type&& action, future_type&& future, promise_type&& promise)
          noexcept:
          _parent (std::forward<_Future> (future)), _action (std::move (action)), _promise (std::move (promise))
        { }
    };
}