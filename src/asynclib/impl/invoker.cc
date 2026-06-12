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
#include <asynclib/impl/invoker.h>
#include <asynclib/impl/slice.h>

struct __invoker_callback
{

  void (*func) (gpointer user_data);
  GDestroyNotify notify;
  gpointer user_data;

  [[gnu::always_inline]] inline void operator()() const noexcept
    { func (user_data); }

  inline ~__invoker_callback () noexcept
    { if (NULL != notify) notify (user_data); }

  inline __invoker_callback (void (*_func) (gpointer), gpointer _user_data, GDestroyNotify _notify = nullptr) noexcept:
    func (_func), notify (_notify), user_data (_user_data) { }
};

struct __invoker_source
{

  GSource parent;
  GAsyncQueue* async_queue;
  GMainContext* owner;
};

[[gnu::always_inline]]
static inline void InvokerSource_dispatch (__invoker_source* source) noexcept
{

  auto callback = (__invoker_callback*) g_async_queue_pop (source->async_queue);
  (*callback) ();

return g_slice_free_<__invoker_callback> (callback);
}

static void remove_invoker (GMainContext* main_context) noexcept;

static GSourceFuncs source_funcs =
{

  .prepare = [](GSource* _p_source, gint* out_timeout) noexcept -> gint
    { return (*out_timeout = 0, (int) FALSE); },

  .check = [](GSource* _p_source) noexcept -> gboolean
    { return g_async_queue_length (((__invoker_source*) _p_source)->async_queue) > 0; },

  .dispatch = [](GSource* _p_source, GSourceFunc func, gpointer user_data) noexcept -> gboolean
    { return (InvokerSource_dispatch ((__invoker_source*) _p_source), G_SOURCE_CONTINUE); },

  .finalize = [](GSource* _p_source) noexcept -> void
    {
      remove_invoker (((__invoker_source*) _p_source)->owner);
      g_async_queue_unref (((__invoker_source*) _p_source)->async_queue);
    },

  .closure_callback = NULL,
  .closure_marshal = NULL,
};

GRWLock invoker_table_lock;

static GHashTable* get_invoker_table () noexcept
{

  static GHashTable* invoker_table = NULL;

  if (g_once_init_enter_pointer (&invoker_table))
    {

      auto table = g_hash_table_new (g_direct_hash, g_direct_equal);
      g_once_init_leave_pointer (&invoker_table, table);
    }
return invoker_table;
}

static __invoker_source* find_invoker (GMainContext* main_context) noexcept
{

  g_rw_lock_reader_lock (&invoker_table_lock);

  auto table = get_invoker_table ();
  auto invoker = g_hash_table_lookup (table, main_context);

  g_rw_lock_reader_unlock (&invoker_table_lock);

  if (G_LIKELY (NULL != invoker))

    return (__invoker_source*) g_source_ref ((GSource*) invoker);
  else
    {

      if (! g_rw_lock_writer_trylock (&invoker_table_lock))
        return find_invoker (main_context);
    }

  auto source = g_source_new (&source_funcs, sizeof (__invoker_source));

  ((__invoker_source*) source)->async_queue = g_async_queue_new_full (g_slice_free_<__invoker_callback>);
  ((__invoker_source*) source)->owner = main_context;

  g_hash_table_insert (table, main_context, source);

  g_source_set_callback (source, NULL, &source_funcs, NULL);
  g_source_set_priority (source, G_PRIORITY_HIGH_IDLE);
  g_source_set_static_name (source, "[Asynclib::invoke_in_context]");
  g_source_attach (source, main_context);

  g_rw_lock_writer_unlock (&invoker_table_lock);

return (__invoker_source*) source;
}

static void remove_invoker (GMainContext* main_context) noexcept
{

  auto table = get_invoker_table ();

  g_rw_lock_writer_lock (&invoker_table_lock);

  g_hash_table_remove (table, main_context);
  g_rw_lock_writer_unlock (&invoker_table_lock);
}

void _asynclib_invoke_in_context (GMainContext* main_context, void (*func)(gpointer), gpointer user_data, GDestroyNotify notify) noexcept
{

  auto invoker = find_invoker (main_context);
  auto callback = g_slice_new_<__invoker_callback> (func, user_data, notify);

  g_async_queue_push (invoker->async_queue, callback);
  g_source_unref ((GSource*) invoker);
}