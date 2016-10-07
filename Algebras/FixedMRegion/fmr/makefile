CC=g++
CPPFLAGS=-O2 -fPIC -Wall -I../include -I/opt/local/include -ggdb -Wno-sign-compare -Wno-reorder
LDFLAGS=-shared
OBJ=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: libfmr.so libfmr.a

libfmr.so: $(OBJ)
	$(CC) -o libfmr.so $(OBJ) $(LDFLAGS)

libfmr.a: $(OBJ)
	ar rcs libfmr.a $(OBJ) 

clean:
	rm  -f *.o *~ libfmr.so libfmr.a
