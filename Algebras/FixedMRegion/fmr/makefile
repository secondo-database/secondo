# Built on its own, without makefile.env, so the compiler is named here -- but
# honour SECONDO_CPP like the rest of the tree. The old -I/opt/local/include
# was a MacPorts path; set FMR_EXTRA_INCLUDES if headers live outside the
# default search path.
CXX ?= g++
ifdef SECONDO_CPP
CXX := $(SECONDO_CPP)
endif
FMR_EXTRA_INCLUDES ?=
CPPFLAGS=-O2 -fPIC -Wall -I../../../include $(addprefix -I,$(FMR_EXTRA_INCLUDES)) -ggdb -Wno-sign-compare -Wno-reorder -I.
LDFLAGS=-shared
OBJ=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: libfmr.so libfmr.a

libfmr.so: $(OBJ)
	$(CXX) -o libfmr.so $(OBJ) $(LDFLAGS)

libfmr.a: $(OBJ)
	$(AR) rcs libfmr.a $(OBJ) 

.PHONY: example
example: libfmr.a example/fmr.o
	$(CC) -o example/fmr example/fmr.o libfmr.a

cadexample: example
	example/fmr mkcad example/$(O) 450

clean:
	rm  -f *.o *~ libfmr.so libfmr.a example/fmr.o example/fmr
