# This library is built on its own and does not include makefile.env, so it
# names its compiler itself. The default stays clang++ because the precompiled
# header below is spelled the clang way (-x c++-header / -include-pch), but
# SECONDO_CPP is honoured like everywhere else in the tree -- and when it
# selects a non-clang compiler the PCH is simply not used.
CXX := clang++
ifdef SECONDO_CPP
CXX := $(SECONDO_CPP)
endif

USE_PCH := $(if $(findstring clang,$(notdir $(CXX))),yes)

CPPFLAGS=-O3 -fPIC -Wall -ggdb -I../../../include
LIBS=-lCGAL -lCGAL_Core -lboost_thread -lgmp -lmpfr
LDFLAGS=$(LIBS)
LIBOBJ=PMRegion_operations.o PMRegion_conversions.o PMRegion_helpers.o RList.o PMRegion_coverduration.o PMRegion_decompose.o

.PHONY: all clean

all: libpmregion.a pmregcli

PMRegion_internal.h.pch: PMRegion_internal.h
	$(CXX) -x c++-header $(CPPFLAGS) $< -o $@

%.o: %.cpp $(if $(USE_PCH),PMRegion_internal.h.pch)
	$(CXX) $(CPPFLAGS) $(if $(USE_PCH),-include-pch PMRegion_internal.h.pch) -c $< -o $@

libpmregion.a: $(LIBOBJ)
	$(RM) $@
	$(AR) -cq $@ $(LIBOBJ)

pmregcli: libpmregion.a pmregcli.o
	$(CXX) -o pmregcli pmregcli.o $(LDFLAGS) libpmregion.a ../../../Tools/Utilities/satof.o

clean:
	$(RM) *.o *~ *.pch* pmregcli libpmregion.a
