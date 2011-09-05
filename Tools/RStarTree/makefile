


include ../../makefile.env

all: $(LIBDIR)/librstartree.a 

$(LIBDIR)/librstartree.a: util.o rstartree.o storage.o 
	$(AR) -rs $(LIBDIR)/librstartree.a util.o rstartree.o storage.o


rstartree.o: rstartree.h rstartree.cc
	$(CPPC) -c -o rstartree.o rstartree.cc
	
util.o: util.h util.cc
	$(CPPC) -c -o util.o util.cc

storage.o: storage.h storage.cc
	$(CPPC) -c -o storage.o storage.cc


clean:
	-rm -f *.o *.a $(LIBDIR)/librstartree.a



