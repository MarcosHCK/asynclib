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

static std::future<void> io_work (const gchar* address, GCancellable* cancellable = nullptr);

int main (int argc, char* argv[])
{

  GMainContext* main_context = g_main_context_ref_thread_default ();
  GMainLoop* main_loop = g_main_loop_new (main_context, FALSE);

  io_work ("localhost") >> [=](std::future<void>& f) noexcept { f.get (); g_main_loop_quit (main_loop); };
  g_main_loop_run (main_loop);

  g_main_context_unref (main_context);
  g_main_loop_unref (main_loop);
return 0;
}

asynclib::gio_task<g_socket_client_connect_async, g_socket_client_connect_finish> g_socket_client_connect_task;

static std::future<GIOStream*> connect_any (GList* addresses, GCancellable* cancellable)
{

  GIOStream* stream = nullptr;
  GSocketClient* socket_client = g_socket_client_new ();

  for (auto link = addresses; NULL != link && NULL == stream; link = link->next)
    {

      auto inet_address = (GInetAddress*) link->data;
      auto socket_address = g_inet_socket_address_new (inet_address, 8000);

      try
        {

          auto connection = co_await g_socket_client_connect_task (socket_client, G_SOCKET_CONNECTABLE (socket_address), cancellable);
          auto address = g_socket_connectable_to_string (G_SOCKET_CONNECTABLE (socket_address));

          g_message ("[%s] connected", address);
          g_free (address);

          stream = G_IO_STREAM (connection);
          break;
        }
      catch (asynclib::gio_error& error)
        {

          auto address = g_socket_connectable_to_string (G_SOCKET_CONNECTABLE (socket_address));
          auto tmperr = error.get_g_error ();

          g_warning ("[%s] connect()!: %s: %u: %s", address, g_quark_to_string (tmperr->domain), tmperr->code, tmperr->message);
          g_free (address);
        }

      g_object_unref (socket_address);
    }
co_return (g_object_unref (socket_client), stream);
}

asynclib::gio_task<g_file_replace_async, g_file_replace_finish> g_file_replace_task;
asynclib::gio_task<g_io_stream_close_async, g_io_stream_close_finish> g_io_stream_close_task;
asynclib::gio_task<g_output_stream_splice_async, g_output_stream_splice_finish> g_output_stream_splice_task;
asynclib::gio_task<g_resolver_lookup_by_name_async, g_resolver_lookup_by_name_finish> g_resolver_lookup_by_name_task;

static std::future<void> io_work (const gchar* address, GCancellable* cancellable)
{

  auto resolver = g_resolver_get_default ();
  auto addresses = co_await g_resolver_lookup_by_name_task (resolver, address, cancellable);
  auto net_stream = co_await connect_any (addresses, cancellable);

  g_resolver_free_addresses (addresses);

  if (G_UNLIKELY (NULL == net_stream))
    throw asynclib::gio_error (G_IO_ERROR, G_IO_ERROR_FAILED, "can't connect to '%s'", address);

  auto file = g_file_new_for_path ("output");
  auto file_stream = co_await g_file_replace_task (file, NULL, FALSE, G_FILE_CREATE_NONE, G_PRIORITY_DEFAULT, cancellable);
  g_object_unref (file);

  auto input_stream = g_io_stream_get_input_stream (net_stream);

  co_await g_output_stream_splice_task (G_OUTPUT_STREAM (file_stream), input_stream, G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET, G_PRIORITY_DEFAULT, cancellable);
  co_await g_io_stream_close_task (net_stream, G_PRIORITY_DEFAULT, cancellable);
  g_object_unref (net_stream);
}