
####### Compiler, tools and options

CC       = gcc
CXX      = g++

CFLAGS   = -pipe -Wall -W -Wno-deprecated -g -fPIC 
CXXFLAGS = -pipe -Wall -W -Wno-deprecated -g -fPIC 

INCPATH  = 
LINK     = g++
LFLAGS   = -static 
LIBS     = -g
AR       = ar cqs
RANLIB   = 

TAR      = tar -cf
GZIP     = gzip -9f
COPY     = cp -f
COPY_FILE= $(COPY)
COPY_DIR = $(COPY) -r
INSTALL_FILE= $(COPY_FILE)
INSTALL_DIR = $(COPY_DIR)
DEL_FILE = rm -f
SYMLINK  = ln -sf
DEL_DIR  = rmdir
MOVE     = mv -f
CHK_DIR_EXISTS= test -d
MKDIR    = mkdir -p

####### Output directory

OBJECTS_DIR = ./

####### Files

INSTALL_ROOT = /home/hs/include

HEADERS = algo.h \
		algo.t \
		argcheck.h \
		datastream.h \
		datastream.t \
		farrexpression.h \
		fixedarray.h \
		fixedarray.t \
		functions.h \
		matrixcheck.h \
		matrix.h \
		matrix.t \
		mexpression.h \
		mindex.h \
		mindex.t \
		mstorage.h \
		mstorage.t
        
SOURCES = functions.cpp
OBJECTS = functions.o
DESTDIR  = 
TARGET   = libschnek.a

first: all
####### Implicit rules

.SUFFIXES: .c .o .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules

all: $(TARGET)

staticlib: $(TARGET)

$(TARGET):  $(OBJECTS) 
	-$(DEL_FILE) $(TARGET) 
	$(AR) $(TARGET) $(OBJECTS) 

dist: 
	@mkdir -p .tmp/schnek && $(COPY_FILE) --parents $(SOURCES) $(HEADERS) $(FORMS) $(DIST) .tmp/schnek/ && ( cd `dirname .tmp/schnek` && $(TAR) schnek.tar schnek && $(GZIP) schnek.tar ) && $(MOVE) `dirname .tmp/schnek`/schnek.tar.gz . && $(DEL_FILE) -r .tmp/schnek


clean:
	-$(DEL_FILE) *~ core *.core *.o


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 


FORCE:

####### Compile

####### Install

install_target: all 
	@$(CHK_DIR_EXISTS) "$(INSTALL_ROOT)/../lib/" || $(MKDIR) "$(INSTALL_ROOT)/../lib/"
	-$(INSTALL_FILE) "$(TARGET)" "$(INSTALL_ROOT)/../lib/$(TARGET)"

uninstall_target: 
	-$(DEL_FILE) "$(INSTALL_ROOT)/../lib/$(TARGET)"


install_incfiles:
	@$(CHK_DIR_EXISTS) "$(INSTALL_ROOT)/schnek/" || $(MKDIR) "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "algo.h" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "algo.t" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "argcheck.h" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "datastream.h" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "datastream.t" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "farrexpression.h" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "fixedarray.h" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "fixedarray.t" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "functions.h" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "matrix.h" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "matrix.t" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "matrixcheck.h" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "mexpression.h" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "mindex.h" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "mindex.t" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "mstorage.h" "$(INSTALL_ROOT)/schnek/"
	-$(INSTALL_FILE) "mstorage.t" "$(INSTALL_ROOT)/schnek/"


uninstall_incfiles: 
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/algo.h"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/algo.t"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/argcheck.h"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/datastream.h"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/datastream.t"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/farrexpression.h"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/fixedarray.h"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/fixedarray.t"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/functions.h"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/matrix.h"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/matrix.t"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/matrixcheck.h"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/mexpression.h"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/mindex.h"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/mindex.t"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/mstorage.h"
	-$(DEL_FILE) -r "$(INSTALL_ROOT)/schnek/mstorage.t"
	-$(DEL_DIR) "$(INSTALL_ROOT)/schnek/"


install: install_target install_incfiles  

uninstall: uninstall_target uninstall_incfiles  

