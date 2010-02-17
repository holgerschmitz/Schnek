
OFLAGS  = -O3
CXXFLAGS = $(OFLAGS)

INCLUDE = -I$(PACKAGE_BASE)/src

COMPILE = $(CXX) $(CXXFLAGS) $(INCLUDE)

SOURCES = testfixarr.cpp testmatrix.cpp test_schneck.cpp

HEADERS = 

OBJECTS = testmatrix.o test_schneck.o

LDFLAGS = 

all: testsuite testfixarr

testsuite: $(OBJECTS)
	$(COMPILE) $(OBJECTS) $(LDFLAGS) -o test_schneck

testfixarr: testfixarr.o
	$(COMPILE) testfixarr.o $(LDFLAGS) -o testfixarr

runtest:
	./testfixarr
#	./test_schneck

%.a : 
	$(AR) $(ARFLAGS) $@ $^


%.o: %.cpp
	$(COMPILE) -c $<

clean:
	-rm -f $(OBJECTS) core test_schneck


