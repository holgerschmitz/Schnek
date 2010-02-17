
export PACKAGE_BASE = $(PWD)

export CXX = g++
export INSTALL = install
export QTINCLUDE = -I /usr/lib/qt-3.3/include/

export PACKAGE_LIB = $(PACKAGE_BASE)/lib
export PACKAGE_SRC = $(PACKAGE_BASE)/src
export PACKAGE_BIN = $(PACKAGE_BASE)/bin

ALLLIBS = 

all: schneck

schneck: $(ALLLIBS)
	cd src && make all

test:
	cd testsuite && make all

runtest:
	cd testsuite && make runtest

install:
	cd src && make install

doc:
	cd src && doxygen

wc:
	-wc -l src/*.h src/*.cpp

.PHONY : clean base_clean

clean: base_clean doc_clean
	
base_clean: 
	cd src && make distclean
	-rm -f core* bin/* lib/lib*

doc_clean:
	cd src && rm -rf doc/


