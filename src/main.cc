/* Copyright (C) 2025-2026 MarcosHCK
 * This file is part of futures.
 *
 * futures is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * futures is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <asynclib/asynclib.h>

static std::future<void> io_work (GCancellable* cancellable = nullptr);

int main (int argc, char* argv[])
{

  GMainContext* main_context = g_main_context_ref_thread_default ();
  GMainLoop* main_loop = g_main_loop_new (main_context, FALSE);

  io_work () >> [=](std::future<void>& f) noexcept { f.get (); g_main_loop_quit (main_loop); };
  g_main_loop_run (main_loop);

  g_main_context_unref (main_context);
  g_main_loop_unref (main_loop);
return 0;
}

static inline std::pair<GFile*, GFileIOStream*> g_file_new_tmp_finish_ (GAsyncResult* result, GError** error)
{

  GFileIOStream* io_stream;
  GFile* file = g_file_new_tmp_finish (result, &io_stream, error);

return std::make_pair (file, io_stream);
}

asynclib::gio_task<g_file_delete_async, g_file_delete_finish> g_file_delete_task;
asynclib::gio_task<g_file_new_tmp_async, g_file_new_tmp_finish_> g_file_new_tmp_task;
asynclib::gio_task<g_io_stream_close_async, g_io_stream_close_finish> g_io_stream_close_task;

static std::future<void> io_work (GCancellable* cancellable)
{

  auto [ file, io_stream ] = co_await g_file_new_tmp_task (NULL, G_PRIORITY_DEFAULT, cancellable);
  g_print ("file = '%s'\n", g_file_peek_path (file));

  co_await g_io_stream_close_task ((GIOStream*) io_stream, G_PRIORITY_DEFAULT, cancellable);
  co_await g_file_delete_task (file, G_PRIORITY_DEFAULT, cancellable);
}