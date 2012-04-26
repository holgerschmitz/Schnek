lib_LTLIBRARIES = libschnek.la

libschnekincludedir = $(includedir)/schnek

libschnek_la_SOURCES = \
  functions.cpp

libschnekinclude_HEADERS = \
  algo.h           \
  algo.t           \
  argcheck.h       \
	datastream.h     \
	datastream.t     \
	farrexpression.h \
	fixedarray.h     \
	fixedarray.t     \
	functions.h      \
	gridcheck.h      \
	grid.h           \
	grid.t           \
	gstorage.h       \
	gstorage.t       \
	typetools.h      \
	vector.h         \
	vector.t
  
libschnek_la_LDFLAGS = -version-info 0:0:0

include parser/Makefile.am

include variables/Makefile.am
