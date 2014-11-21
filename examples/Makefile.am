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


EXTRA_PROGRAMS = \
  example_grids_basics \
  example_grids_cpc \
	example_array \
  example_array_functions \
  example_array_cpc \
	example_setup_first \
	example_setup_errors \
	example_setup_defaults \
  example_setup_constant \
  example_setup_array \
  example_setup_cpc \
  example_setup_blocks_cpc \
  example_setup_updater_cpc \
	example_blocks_hierarchy \
  example_literature_cpc \
	example_range \
  example_range_cpc \
  example_subgrid_cpc \
  example_gridtransform_cpc \
  example_fields_cpc \
  example_mpisub_cpc \
	example_boundary

example_grids_basics_SOURCES = example_grids_basics.cpp
example_grids_basics_LDADD = -L../src -lschnek

example_grids_cpc_SOURCES = example_grids_cpc.cpp
example_grids_cpc_LDADD = -L../src -lschnek

example_array_SOURCES = example_array.cpp
example_array_LDADD = -L../src -lschnek

example_array_functions_SOURCES = example_array_functions.cpp
example_array_functions_LDADD = -L../src -lschnek

example_array_cpc_SOURCES = example_array_cpc.cpp
example_array_cpc_LDADD = -L../src -lschnek

example_setup_first_SOURCES = example_setup_first.cpp
example_setup_first_LDADD = -L../src -lschnek

example_setup_errors_SOURCES = example_setup_errors.cpp
example_setup_errors_LDADD = -L../src -lschnek

example_setup_defaults_SOURCES = example_setup_defaults.cpp
example_setup_defaults_LDADD = -L../src -lschnek

example_setup_constant_SOURCES = example_setup_constant.cpp
example_setup_constant_LDADD = -L../src -lschnek

example_setup_array_SOURCES = example_setup_array.cpp
example_setup_array_LDADD = -L../src -lschnek

example_setup_cpc_SOURCES = example_setup_cpc.cpp
example_setup_cpc_LDADD = -L../src -lschnek

example_setup_blocks_cpc_SOURCES = example_setup_blocks_cpc.cpp
example_setup_blocks_cpc_LDADD = -L../src -lschnek

example_setup_updater_cpc_SOURCES = example_setup_updater_cpc.cpp
example_setup_updater_cpc_LDADD = -L../src -lschnek

example_blocks_hierarchy_SOURCES = example_blocks_hierarchy.cpp
example_blocks_hierarchy_LDADD = -L../src -lschnek

example_literature_cpc_SOURCES = example_literature_cpc.cpp
example_literature_cpc_LDADD = -L../src -lschnek

example_range_SOURCES = example_range.cpp
example_range_LDADD = -L../src -lschnek

example_range_cpc_SOURCES = example_range_cpc.cpp
example_range_cpc_LDADD = -L../src -lschnek

example_subgrid_cpc_SOURCES = example_subgrid_cpc.cpp
example_subgrid_cpc_LDADD = -L../src -lschnek

example_gridtransform_cpc_SOURCES = example_gridtransform_cpc.cpp
example_gridtransform_cpc_LDADD = -L../src -lschnek

example_fields_cpc_SOURCES = example_fields_cpc.cpp
example_fields_cpc_LDADD = -L../src -lschnek

example_mpisub_cpc_SOURCES = example_mpisub_cpc.cpp
example_mpisub_cpc_LDADD = -L../src -lschnek

example_boundary_SOURCES = example_boundary.cpp
example_boundary_LDADD = -L../src -lschnek

SCHNEK_EXAMPLES_OBJ = \
  example_grids_basics$(EXEEXT) \
  example_grids_cpc$(EXEEXT) \
	example_array$(EXEEXT) \
  example_array_functions$(EXEEXT) \
  example_array_cpc$(EXEEXT) \
  example_setup_first$(EXEEXT) \
	example_setup_errors$(EXEEXT) \
	example_setup_defaults$(EXEEXT) \
  example_setup_constant$(EXEEXT) \
  example_setup_array$(EXEEXT) \
  example_setup_cpc$(EXEEXT) \
  example_setup_blocks_cpc$(EXEEXT) \
  example_setup_updater_cpc$(EXEEXT) \
	example_blocks_hierarchy$(EXEEXT) \
  example_literature_cpc$(EXEEXT) \
	example_range$(EXEEXT) \
  example_range_cpc$(EXEEXT) \
  example_subgrid_cpc$(EXEEXT) \
  example_gridtransform_cpc$(EXEEXT) \
  example_fields_cpc$(EXEEXT) \
  example_mpisub_cpc$(EXEEXT) \
	example_boundary$(EXEEXT)

examples: $(SCHNEK_EXAMPLES_OBJ)
	
.PHONY: bench 