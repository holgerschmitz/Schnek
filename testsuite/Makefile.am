
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
	util/test_hilbert.cpp

test_parser_SOURCES = test_parser.cpp

test_parser_LDADD = -L../src -lschnek
	
schnek_test_HEADERS = \
	utility.hpp
