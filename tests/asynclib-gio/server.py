# Copyright (C) 2025-2026 MarcosHCK
# This file is part of asynclib.
#
# asynclib is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# asynclib is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#
from argparse import ArgumentParser
from http.server import HTTPServer, BaseHTTPRequestHandler
import os, re, sys

def parse_size (size_str):

  units = {
    'B': 1, 'KB': 1000, 'MB': 1000**2, 'GB': 1000**3, 'TB': 1000**4,
    'KiB': 1024, 'MiB': 1024**2, 'GiB': 1024**3, 'TiB': 1024**4,
    'K': 1000, 'M': 1000**2, 'G': 1000**3  # shorthand
  }

  if not (match := re.match (r'([\d.]+)\s*([A-Za-iK-Z]+)?', size_str.strip ())):
    raise ValueError (f'Invalid size format: {size_str}')

  value, unit = match.groups ()

  if not (factor := units.get (unit or 'B', None)):
    raise ValueError (f'Invalid size format: {size_str}')

  return int (factor * float (value))

if __name__ == '__main__':

  parser = ArgumentParser ('http_server')

  parser.add_argument ('-b', '--bind', default= '', type = str)
  parser.add_argument ('-p', '--port', default = 8000, type = int)
  parser.add_argument ('-s', '--size', default = '1MiB', type = str)

  args = parser.parse_args ()
  blob = os.urandom (parse_size (args.size))

  class Handler (BaseHTTPRequestHandler):

    def do_GET (self):

      self.send_response (200)
      self.send_header ('Content-Type', 'text/plain')
      self.end_headers ()
      self.wfile.write (blob)

  print (f'HTTPServer (({args.bind}, {args.port}), Handler).serve_forever ()')
  sys.stdout.close ()

  HTTPServer ((args.bind, args.port), Handler).serve_forever ()