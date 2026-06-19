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
#include <asynclib-gio/asynclib-gio.h>
#include <tests/testing.h>
using namespace std::chrono_literals;
using namespace testing;

static std::pair<GFile*, GIOStream*> g_file_new_tmp_finish_ (GAsyncResult* result, GError** error)
{

  GFileIOStream* io_stream;
  GFile* file = g_file_new_tmp_finish (result, &io_stream, error);

return std::make_pair (file, G_IO_STREAM (io_stream));
}

asynclib::async_function g_file_delete_task (g_file_delete_async, g_file_delete_finish);
asynclib::async_function g_file_new_tmp_task (g_file_new_tmp_async, g_file_new_tmp_finish_);
asynclib::async_function g_io_stream_close_task (g_io_stream_close_async, g_io_stream_close_finish);

static asynclib::async_task<bool> io_work (GCancellable* cancellable)
{

  auto [ file, io_stream ] = co_await g_file_new_tmp_task (NULL, G_PRIORITY_DEFAULT, cancellable);
  // auto [ file, io_stream ] = co_await g_file_new_tmp_task (NULL, G_PRIORITY_DEFAULT, cancellable);

  auto exists = g_file_test (g_file_peek_path (file), G_FILE_TEST_EXISTS);

  // co_await g_io_stream_close_task ((GIOStream*) io_stream, G_PRIORITY_DEFAULT, cancellable);
  co_await g_file_delete_task (file, G_PRIORITY_DEFAULT, cancellable);
co_return exists;
}

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_ (TESTPATHROOT "/works", []
    {

      bool r = false;
      guint ready = 0;

      auto task = io_work (NULL);

      struct D { decltype (task.end) end; decltype (r)* r; decltype (ready)* ready; }
           d = { .end = task.end, .r = &r, .ready = &ready };

      task.begin ([](GObject*, GAsyncResult* result, gpointer user_data)
        {

          auto p = (D*) user_data;
          auto e = (GError*) nullptr;
          auto r = p->end (result, &e);
          *p->r = r;

          g_assert_no_error (e);
          g_atomic_int_set (p->ready, 1);
        }, &d);

      for (auto context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&ready); )
        g_main_context_iteration (context, FALSE);

      g_assert_true (r);
    });

return g_test_run ();
}