CC=g++
CPPFLAGS=-O2 -fPIC -Wall -I../include -I/opt/local/include -ggdb -Wno-sign-compare -Wno-reorder -I.
LDFLAGS=-shared
OBJ=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: libfmr.so libfmr.a

libfmr.so: $(OBJ)
	$(CC) -o libfmr.so $(OBJ) $(LDFLAGS)

libfmr.a: $(OBJ)
	ar rcs libfmr.a $(OBJ) 

.PHONY: example
example: libfmr.a example/fmr.o
	$(CC) -o example/fmr example/fmr.o libfmr.a

clean:
	rm  -f *.o *~ libfmr.so libfmr.a example/fmr.o example/fmr
