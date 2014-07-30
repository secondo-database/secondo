CC=g++
CPPFLAGS=-O2 -I../include
OBJ=rip.o
LIBS=../lib/librip.so
LDFLAGS=$(LIBS)


all: rip test

rip: $(OBJ)
	$(CC) -o rip $(OBJ) $(LDFLAGS)

clean:
	rm -f *.o *~ rip

test: rip
	@echo
	@echo =====================
	@echo Performing self-tests
	@echo =====================
	@echo
	./rip objects/box1 objects/box2 "2014-01-01 00:00" "2014-01-02 00:00" distance
	@echo
	./rip objects/box1 objects/box2 "2014-01-01 00:00" "2014-01-02 00:00" Overlap:10
