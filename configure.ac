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

AC_INIT([Schnek],[0.1])

AC_CONFIG_SRCDIR([src/grid.hpp])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([src/config.hpp])
AM_INIT_AUTOMAKE
# VERSION="0.1"
# PACKAGE="schnek"

AC_LANG(C++)

AC_SUBST(INTI_CFLAGS)
AC_SUBST(INTI_LIBS)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL

AX_BOOST_BASE([1.48.0],,[AC_MSG_ERROR([Schnek needs Boost version 1.48.0 to compile!])])

# Search for MPI support
# allows the  --with-mpi option (yes, no, auto)
AC_ARG_WITH(mpi, [AS_HELP_STRING([--with-mpi],
           [compile with MPI (parallelization) support. If none is found,
           MPI is not used. Default: auto])
       ],,[with_mpi=auto])
       
if test x"$with_mpi" != xno; then
  CXXFLAGS_save=$CXXFLAGS
  LDFLAGS_save=$LDFLAGS
  CXXFLAGS=""
  LDFLAGS=""
  AX_MPI([
    MPI_CXXFLAGS=`$MPICXX --showme:compile`
    [echo Using MPI_CXXFLAGS=$MPI_CXXFLAGS]
    AC_SUBST(MPI_CXXFLAGS)
    MPI_LDFLAGS=`$MPICXX --showme:link`
    [echo Using MPI_LDFLAGS=$MPI_LDFLAGS]
    AC_SUBST(MPI_LDFLAGS)
    CXXFLAGS="$CXXFLAGS_save $MPI_CXXFLAGS"
    LDFLAGS="$LDFLAGS_save $MPI_LDFLAGS"
  ],[
    if test x"$with_mpi" = xyes; then
      AC_MSG_ERROR([MPI support requested but no MPI library found!])
    fi
  ])
fi

# Search for HDF5 libraries
# To distinguish between serial and parallel, use the H5_HAVE_PARALLEL macro
# supplied by HDF5 itself
# Checks for libraries.
# start: Check for HDF5 support
AC_ARG_WITH([hdf5],
            [AS_HELP_STRING([--with-hdf5=path], [location of the hdf5 library])],[
              hdf5_LIBS="-L$withval/lib -lhdf5 -lhdf5_cpp"
              hdf5_CFLAGS="-I$withval/include"
            ],
            [with_hdf5=auto]
           )


if test x"$with_hdf5" = xyes; then
  hdf5_LIBS="-lhdf5 -lhdf5_cpp"
  hdf5_CFLAGS=""
fi

if test x"$with_hdf5" = xauto; then
  hdf5_LIBS="-lhdf5 -lhdf5_cpp"
  hdf5_CFLAGS=""
fi

if test x"$with_hdf5" = xno; then
  hdf5_LIBS=""
  hdf5_CFLAGS=""
fi

AC_SUBST([hdf5_CFLAGS])
AC_SUBST([hdf5_LIBS])

AC_ARG_WITH([hdf5-include],
            [AS_HELP_STRING([--with-hdf5-include=path], [location of the hdf5 headers, defaults to /usr/include])],[ 
                hdf5_CFLAGS="-I$withval"
                AC_SUBST([hdf5_CFLAGS])
                ],
            [])

AC_ARG_WITH([hdf5-lib],
            [AS_HELP_STRING([--with-hdf5-lib=path], [location of the hdf5 libraries])], [
                hdf5_LIBS="-L$withval -lhdf5 -lhdf5_cpp"
                AC_SUBST([hdf5_LIBS])
            ],
            [])


PKG_CHECK_MODULES([hdf5], [hdf5 >= 1.8.0],[
                    AC_SUBST([hdf5_CFLAGS])
                    AC_SUBST([hdf5_LIBS])
                    ],
                    [
                    hdf5_LIBS="-L/usr/lib -lhdf5 -lhdf5_cpp"
                    hdf5_CFLAGS="-I/usr/include"
                    AC_MSG_WARN(["No hdf5 >= 1.8.0 found automatically. Check your \$PKG_CONFIG_PATH or set --with-hdf5 flag. Assuming hdf5 is stored in /usr "])
                    ]
                  )


if test x"$with_hdf5" != xno; then
  AC_LANG_PUSH([C++])
  CPPFLAGS_save=$CPPFLAGS
  LDFLAGS_save=$LDFLAGS
  CPPFLAGS=$hdf5_CFLAGS
  LDFLAGS=$hdf5_LIBS
  
  found_hdf5=yes;
  AC_CHECK_HEADER([hdf5.h],,[found_hdf5=no])
  AC_CHECK_LIB(hdf5, H5Fflush,,[found_hdf5=no])
  AC_CHECK_LIB(hdf5_hl, H5DSis_scale,,[found_hdf5=no])

  CPPFLAGS=$CPPFLAGS_save
  LDFLAGS=$LDFLAGS_save
  AC_LANG_POP([C++])
  
  if test $found_hdf5 = yes; then
    AC_DEFINE([HAVE_HDF5],[1],[Define this macro if Schnek is compiled with HDF5 support])
  else if test x"$with_hdf5" = xyes; then
      AC_MSG_ERROR([HDF5 support requested HDF5 library not found!])
    fi
  fi

fi



AC_LANG_PUSH([C++])
OLDCPPFLAGS=$CPPFLAGS
OLDLDFLAGS=$LDFLAGS
CPPFLAGS=$hdf5_CFLAGS
LDFLAGS=$hdf5_LIBS

CPPFLAGS=$OLDCPPFLAGS
LDFLAGS=$OLDLDFLAGS
AC_LANG_POP([C++])
#End :: Check HDF5 suport


       
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
                 testsuite/Makefile])
AC_OUTPUT
