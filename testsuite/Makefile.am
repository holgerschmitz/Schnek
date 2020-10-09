
AUTOMAKE_OPTIONS = subdir-objects

check_PROGRAMS = \
  schnek_test
  
TESTS = \
  schnek_test
  
AM_CXXFLAGS = -I../src

schnek_testdir = $(includedir)/schnek/testsuite

schnek_test_LDADD = -L../src -lschnek

schnek_test_SOURCES = \
	main.cpp \
	test_array.cpp \
	test_arrayexpression.cpp \
	test_grid.cpp \
	test_parser.cpp \
	test_range.cpp \
	utility.cpp \
	decomposition/mpi_test_context.cpp \
	decomposition/test_mpi_cartesian_decomposition.cpp \
	util/test_hilbert.cpp
	
schnek_test_HEADERS = \
	utility.hpp
