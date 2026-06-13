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

gio_dep = dependency ('gio-2.0',required: true)

__asynclib_gio_headers = [ 'gioerror.h', 'task.h', 'taskactivate.h', 'taskcomplete.h', 'taskfunction.h' ]
__asynclib_gio_sources = [ 'gioerror.cc', 'task.cc' ]

__asynclib_gio_lib = library ('asynclib-gio',
  c_args: [ '-DG_LOG_DOMAIN="Asynclib.GIO"', '-DG_LOG_USE_STRUCTURED=1',
            '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
            '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  cpp_args: [ '-DG_LOG_DOMAIN="Asynclib.GIO"', '-DG_LOG_USE_STRUCTURED=1',
              '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
              '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  dependencies: [ config_dep, gio_dep, __asynclib_dep ],
  sources: [ __asynclib_gio_headers, __asynclib_gio_sources ])

__asynclib_gio_dep = declare_dependency (dependencies: [ gio_dep, __asynclib_dep ], link_with: __asynclib_gio_lib)

meson.override_dependency ('asynclib-gio', __asynclib_gio_dep)

install_headers (__asynclib_gio_headers, subdir: 'asynclib-gio')