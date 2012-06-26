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

include parser/Makefile.am

include variables/Makefile.am
