
check_PROGRAMS = \
  schnek_test \
  test_fixarr \
  test_parser
  
TESTS = \
  schnek_test \
  test_fixarr \
  test_parser

AM_CXXFLAGS = -I../src

schnek_test_SOURCES = \
	main.cpp \
	test_grid.cpp

test_fixarr_SOURCES = test_fixarr.cpp

test_parser_SOURCES = test_parser.cpp

test_parser_LDADD = -L../src -lschnek