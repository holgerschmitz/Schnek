# Makefile.am
#
# Created on: 08 Jul 2014
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


EXTRA_PROGRAMS = example_grids_basics \
	example_setup_first \
	example_range

example_grids_basics_SOURCES = example_grids_basics.cpp
example_grids_basics_LDADD = -L../src -lschnek

example_setup_first_SOURCES = example_setup_first.cpp
example_setup_first_LDADD = -L../src -lschnek

example_range_SOURCES = example_range.cpp
example_range_LDADD = -L../src -lschnek

SCHNEK_EXAMPLES_OBJ = example_grids_basics$(EXEEXT) \
	example_setup_first$(EXEEXT) \
	example_range$(EXEEXT)

examples: $(SCHNEK_EXAMPLES_OBJ)
	
.PHONY: bench 