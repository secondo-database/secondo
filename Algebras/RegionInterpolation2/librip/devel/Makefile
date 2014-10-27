CC=g++
CPPFLAGS=-O2 -fPIC -Wall -I../include -I/opt/local/include -ggdb
LIBS=-llua5.1
LDFLAGS=$(LIBS) -L/opt/local/lib -shared
OBJ=Face.o interpolate.o Interval.o lua.o matchFaces.o MFace.o MFaces.o MSeg.o MSegs.o RList.o ointersect.o Pt.o RotatingPlane.o Seg.o Matches.o Merge.o Poly2.o

all: librip.so librip.a
	cp *.o ../build

librip.so: $(OBJ)
	$(CC) -o librip.so $(OBJ) $(LDFLAGS)
	cp librip.so ../build

librip.a: $(OBJ)
	ar rcs librip.a $(OBJ) 
	cp librip.a ../build

clean:
	rm  -f *.o *~ librip.so librip.a
