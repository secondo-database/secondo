CC=g++
CPPFLAGS=-O2 -fPIC -Wall -I../include
LIBS=-llua5.1
LDFLAGS=$(LIBS) -shared
OBJ=Face.o interpolate.o Interval.o lua.o matchFaces.o MFace.o MFaces.o MSeg.o MSegs.o RList.o ointersect.o Poly.o Pt.o RotatingPlane.o Seg.o

all: librip.so librip.a

librip.so: $(OBJ)
	$(CC) -o librip.so $(OBJ) $(LDFLAGS)

librip.a: $(OBJ)
	ar rcs librip.a $(OBJ) 

clean:
	rm  -f *.o *~ librip.so librip.a
