
check_PROGRAMS = \
  schnek_test \
  test_parser
  
TESTS = \
  schnek_test \
  test_parser

AM_CXXFLAGS = -I../src

schnek_test_SOURCES = \
	main.cpp \
	utility.cpp \
	test_grid.cpp \
	test_array.cpp \
	util/test_hilbert.cpp

test_parser_SOURCES = test_parser.cpp

test_parser_LDADD = -L../src -lschnek