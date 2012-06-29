# Makefile.am
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

lib_LTLIBRARIES = libschnek.la

libschnekincludedir = $(includedir)/schnek

libschnek_la_SOURCES = \
  functions.cpp

libschnekinclude_HEADERS = \
  algo.hpp           \
  algo.t             \
  argcheck.hpp       \
	datastream.hpp     \
	datastream.t       \
	farrexpression.hpp \
	fixedarray.hpp     \
	fixedarray.t       \
	functions.hpp      \
	gridcheck.hpp      \
	grid.hpp           \
	grid.t             \
	gstorage.hpp       \
	gstorage.t         \
	typetools.hpp      \
	vector.hpp         \
	vector.t
  
libschnek_la_LDFLAGS = -version-info 0:0:0

