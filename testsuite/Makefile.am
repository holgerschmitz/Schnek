check_PROGRAMS = testgrid testfixarr test_parser

AM_CXXFLAGS = -I../src $(INTI_CFLAGS)

testgrid_SOURCES = testgrid.cpp
testgrid_LDADD = $(INTI_LIBS)

testfixarr_SOURCES = testfixarr.cpp
testfixarr_LDADD = $(INTI_LIBS)

test_parser_SOURCES = test_parser.cpp
test_parser_LDADD = $(INTI_LIBS) -L../src -lschnek