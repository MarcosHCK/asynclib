# Copyright (C) 2025-2026 MarcosHCK
# This file is part of futures.
#
# futures is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# futures is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

subdir ('impl')

__asynclib_headers = [ 'asynclib.h', 'future.h', 'future_co.h', 'future_op.h' ]
__asynclib_sources = [ 'future.cc' ]

__asynclib_lib = library ('asynclib',
  c_args: [ '-DG_LOG_DOMAIN="Asynclib"', '-DG_LOG_USE_STRUCTURED=1',
            '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
            '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  cpp_args: [ '-DG_LOG_DOMAIN="Asynclib"', '-DG_LOG_USE_STRUCTURED=1',
              '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
              '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  dependencies: [ config_dep, __asynclib_impl_dep ],
  sources: [ __asynclib_headers, __asynclib_sources ])

__asynclib_dep = declare_dependency (link_with: __asynclib_lib)

meson.override_dependency ('asynclib', __asynclib_dep)

install_headers (__asynclib_headers, subdir: 'asynclib')