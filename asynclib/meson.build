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

glib_dep = dependency ('glib-2.0', required: true)

__asynclib_headers = [ 'asynclib.h', 'error.h', 'pollablehost.h' ]
__asynclib_sources = [ 'error.cc', 'pollablehost.cc' ]

foreach bit: [ '', 'awaitable', 'begin', 'end', 'invocation' ]

  __asynclib_headers += [ 'asyncfunction@0@.h'.format (bit) ]
endforeach

foreach bit: [ '', 'coroutine' ]

  __asynclib_headers += [ 'asynctask@0@.h'.format (bit) ]
endforeach

foreach bit: [ 'awaitable', 'chain', 'concepts', 'coroutine', 'pollable' ]

  __asynclib_headers += [ 'future@0@.h'.format (bit) ]
endforeach

__asynclib_lib = library ('asynclib',
  c_args: [ '-DG_LOG_DOMAIN="Asynclib"', '-DG_LOG_USE_STRUCTURED=1',
            '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
            '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  cpp_args: [ '-DG_LOG_DOMAIN="Asynclib"', '-DG_LOG_USE_STRUCTURED=1',
              '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
              '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  dependencies: [ config_dep, glib_dep ],
  sources: [ __asynclib_headers, __asynclib_sources ])

__asynclib_dep = declare_dependency (link_with: __asynclib_lib)

meson.override_dependency ('asynclib', __asynclib_dep)

import ('pkgconfig').generate (__asynclib_lib,
  libraries: [ __asynclib_dep ],
  subdirs: '@0@'.format (meson.project_name ()),
  version: meson.project_version ())

install_headers (__asynclib_headers, subdir: '@0@'.format (meson.project_name ()))