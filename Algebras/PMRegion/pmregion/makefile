CXX=clang++
CPPFLAGS=-O3 -fPIC -Wall -ggdb -I../../../include
LIBS=-lCGAL -lCGAL_Core -lboost_thread -lboost_system -lgmp -lmpfr
LDFLAGS=$(LIBS)
LIBOBJ=PMRegion_operations.o PMRegion_conversions.o PMRegion_helpers.o RList.o PMRegion_coverduration.o

all: libpmregion.a pmregcli

PMRegion_internal.h.pch: PMRegion_internal.h
	clang++ -x c++-header $(CPPFLAGS) PMRegion_internal.h -o PMRegion_internal.h.pch

%.o: %.cpp PMRegion_internal.h.pch
	$(CXX) $(CPPFLAGS) -include-pch PMRegion_internal.h.pch -c $< -o $@

libpmregion.a: $(LIBOBJ)
	rm -f $@
	ar -cq $@ $(LIBOBJ)

pmregcli: libpmregion.a pmregcli.o
	$(CXX) -o pmregcli pmregcli.o $(LDFLAGS) libpmregion.a ../../../Tools/Utilities/satof.o

clean:
	rm  -f *.o *~ *.pch* pmregcli libpmregion.a
