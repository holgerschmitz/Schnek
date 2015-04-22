# configure.ac
#
# Created on: 12 Apr 2007
# Author: Holger Schmitz
# Email: holger@notjustphysics.com
#
# Copyright 2012 Holger Schmitz
#
# This file is part of Schnek.
#
# Schnek is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Schnek is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Schnek.  If not, see <http://www.gnu.org/licenses/>.
 
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([Schnek],[1.0.1])

AC_CONFIG_SRCDIR([src/grid.hpp])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([src/config.hpp src/schnek_config.hpp:src/schnek_config.hpp.in])
  
AM_INIT_AUTOMAKE
VERSION="1.0.1"
PACKAGE="schnek"

AC_LANG(C++)

AC_SUBST(INTI_CFLAGS)
AC_SUBST(INTI_LIBS)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

LT_INIT
AC_SUBST([LIBTOOL_DEPS])

AC_LANG_PUSH([C++])

AX_BOOST_BASE([1.48.0],,[AC_MSG_ERROR([Schnek needs Boost version 1.48.0 to compile!])])

CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"

# Search for MPI support
# allows the  --with-mpi option (yes, no, auto)
AC_ARG_WITH(mpi, [AS_HELP_STRING([--with-mpi],
           [compile with MPI (parallelization) support. If none is found,
           MPI is not used. Default: auto])
       ],,[with_mpi=auto])
       
if test x"$with_mpi" != xno; then
  AX_MPI([
    using_mpi=yes
    CXX=$MPICXX
    CC=$MPICXX
    CPP=$MPICXX
    AC_DEFINE([SCHNEK_HAVE_MPI],[1],[Define this macro if Schnek is compiled with MPI support])
  ],[
    using_mpi=no
    if test x"$with_mpi" = xyes; then
      AC_MSG_ERROR([MPI support requested but no MPI library found!])
    fi
  ])
else
  using_mpi=no
fi

# Search for HDF5 libraries
# To distinguish between serial and parallel, use the H5_HAVE_PARALLEL macro
# supplied by HDF5 itself
# Checks for libraries.
# start: Check for HDF5 support

AX_HDF5([
  AC_DEFINE([SCHNEK_HAVE_HDF5],[1],[Define this macro if Schnek is compiled with HDF5 support])
  EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS $HDF5_CFLAGS"
  EXTRA_LDFLAGS="$EXTRA_LDFLAGS $HDF5_LDFLAGS"
  EXTRA_LIBS="$EXTRA_LIBS $HDF5_LIBS"
],[
  AC_MSG_WARN([Couldn't find the HDF5 library!!  Switching to --without-hdf5.])
])

AC_ARG_ENABLE([parallel_hdf5],
          [AS_HELP_STRING([--enable-parallel-hdf5], [use parallel file access for HDF5 (mpio) if available.])], [])

AS_IF([test "x$enable_parallel_hdf5" != "xno"], [ 
    AC_DEFINE([SCHNEK_USE_HDF_PARALLEL],[1],[Defined if we use parallel file access for HDF5 (mpio) if available.])
  ])
  

CPPFLAGS="$CPPFLAGS $EXTRA_CPPFLAGS"
LDFLAGS="$LDFLAGS $EXTRA_LDFLAGS"
LIBS="$LIBS $EXTRA_LIBS"

#End :: Check HDF5 suport


schnek_lib_version_major=`expr $VERSION : '\([[0-9]]*\)'`
schnek_lib_version_minor=`expr $VERSION : '[[0-9]]*\.\([[0-9]]*\)'`
schnek_lib_version_sub_minor=`expr $VERSION : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
    
schnek_lib_version_long=`expr $schnek_lib_version_major \* 100000 \+  $schnek_lib_version_minor \* 100 \+ $schnek_lib_version_sub_minor`
    
AC_DEFINE_UNQUOTED([SCHNEK_VERSION], [$schnek_lib_version_long],
  [Integer Schnek Version Number])
       
# AC_ARG_PROGRAM

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_HEADER_STDC

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 testsuite/Makefile
                 examples/Makefile])
AC_OUTPUT
