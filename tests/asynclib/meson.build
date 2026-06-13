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

c_args = [ '-DG_LOG_DOMAIN="Asynclib.Tests"', '-DG_LOG_USE_STRUCTURED=1',
           '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
           '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()), ]

testpath = meson.current_source_dir ().replace (basepath + '/', '')

test ((testpath + '/future_co').replace ('/', '.'),
      executable ('future_co',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'future_co') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'future_co') ],
            build_by_default: false,
            dependencies: [ config_dep, __asynclib_dep, __testing_dep ],
            sources: 'future_co.cc'),
      env: [ 'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()) ], )

test ((testpath + '/future_op').replace ('/', '.'),
      executable ('future_op',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'future_op') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'future_op') ],
            build_by_default: false,
            dependencies: [ config_dep, __asynclib_dep, __testing_dep ],
            sources: 'future_op.cc'),
      env: [ 'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()) ], )