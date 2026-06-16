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
#include <algorithm>
#include <functional>
#include <glib.h>
#include <ranges>
#include <vector>

namespace testing
{

  namespace details
    {

      template<typename T, typename... Args>
        concept __test_function_action = std::is_invocable_r_v<void, T, Args...>;

      template<typename T>
        concept __test_function_constructible_fixture = std::is_default_constructible_v<T>;

      template<typename T>
        concept __test_function_destructible_fixture = std::is_destructible_v<T>;

      template<typename T>
        concept __test_function_trivial_fixture = __test_function_constructible_fixture<T>
                                               && __test_function_destructible_fixture<T>;
    }

  void g_test_add_function (const char* path, std::move_only_function<void ()>&& func);

  template<details::__test_function_action Fn>
    requires (std::is_constructible_v<std::move_only_function<void ()>, Fn>)
  static inline void g_test_add_ (const char* path, Fn&& action)
    {
      g_test_add_function (path, std::move_only_function<void ()> (std::forward<Fn> (action)));
    }

  template<details::__test_function_trivial_fixture Ft,
           details::__test_function_action<const Ft&> Fn>
  static inline void g_test_add_ (const char* path, Fn&& action)
    {

      auto func = [action = std::forward<Fn> (action)] ()
        { auto fixture = Ft (); action (fixture); };

      g_test_add_function (path, std::move_only_function<void ()> (func));
    }

  template<details::__test_function_destructible_fixture Ft,
           details::__test_function_action<const Ft&> Fn,
           typename _Setup>
    requires (std::is_invocable_r_v<Ft, _Setup>)
  static inline void g_test_add_ (const char* path, Fn&& action, _Setup&& setup)
    {

      auto func = [action = std::forward<Fn> (action), setup = std::forward<_Setup> (setup)] ()
        { auto fixture = setup (); action (fixture); };

      g_test_add_function (path, std::move_only_function<void ()> (func));
    }

  template<details::__test_function_constructible_fixture Ft,
           details::__test_function_action<const Ft&> Fn,
           typename _Destruct>
    requires (std::is_invocable_r_v<void, _Destruct, Ft&>)
  static inline void g_test_add_ (const char* path, Fn&& action, _Destruct&& destruct)
    {

      auto func = [action = std::forward<Fn> (action), destruct = std::forward<_Destruct> (destruct)] ()
        { auto fixture = Ft (); action (fixture); destruct (fixture); };

      g_test_add_function (path, std::move_only_function<void ()> (func));
    }

  template<typename Ft,
           details::__test_function_action<const Ft&> Fn,
           typename _Setup, typename _Destruct>
    requires (std::is_invocable_r_v<Ft, _Setup>
           && std::is_invocable_r_v<void, _Destruct, Ft&>)
  static inline void g_test_add_ (const char* path, Fn&& action, _Setup&& setup, _Destruct&& destruct)
    {

      auto func = [action = std::forward<Fn> (action), destruct = std::forward<_Destruct> (destruct), setup = std::forward<_Setup> (setup)] ()
        { auto fixture = setup (); action (fixture); destruct (fixture); };

      g_test_add_function (path, std::move_only_function<void ()> (std::forward<Fn> (action)));
    }

# define g_assert_cmp(s1,cmp,s2) (G_GNUC_EXTENSION ({ \
 ; \
      const auto& __s1 = ((s1)); \
      const auto& __s2 = ((s2)); \
 ; \
      if (false == (__s1 cmp __s2)) \
        g_assertion_message (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, #s1 " " #cmp " " #s2); \
    }))

  void g_test_analyze_times (const std::vector<gdouble>& times) noexcept;

  template<std::ranges::view Range>
    requires (std::convertible_to<std::ranges::range_value_t<Range>, double>)
  [[gnu::always_inline]]
  static inline void g_test_analyze_times_unsorted (Range&& range)
    {

      std::vector<double> sorted;

      sorted.assign_range (std::move (range));
      std::sort (sorted.begin (), sorted.end ());

    return g_test_analyze_times (sorted);
    }

  void g_test_save_times (const std::vector<double>& times) noexcept;

  template<std::ranges::view Range>
    requires (std::convertible_to<std::ranges::range_value_t<Range>, double>)
  [[gnu::always_inline]]
  static inline void g_test_save_times (Range&& range)
    {

      std::vector<double> ar;
      ar.assign_range (std::forward<Range> (range));

    return g_test_save_times (ar);
    }
}