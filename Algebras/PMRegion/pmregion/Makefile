CXX=g++
CPPFLAGS=-O3 -fPIC -Wall -ggdb
LIBS=-lCGAL -lCGAL_Core -lboost_thread -lboost_system -lgmp -lmpfr
LDFLAGS=$(LIBS)
LIBOBJ=PMRegion_operations.o PMRegion_conversions.o PMRegion_helpers.o RList.o PMRegion_coverduration.o

all: libpmregion.a pmregcli

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

libpmregion.a: $(LIBOBJ)
	rm -f $@
	ar -cq $@ $(LIBOBJ)

pmregcli: libpmregion.a pmregcli.o
	$(CXX) -o pmregcli pmregcli.o $(LDFLAGS) libpmregion.a 

clean:
	rm  -f *.o *~ *.pch* pmregcli libpmregion.a
