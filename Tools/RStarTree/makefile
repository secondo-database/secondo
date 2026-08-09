


include ../../makefile.env

all: $(LIBDIR)/librstartree.a 

$(LIBDIR)/librstartree.a: util.o rstartree.o storage.o 
	$(AR) -rs $(LIBDIR)/librstartree.a util.o rstartree.o storage.o


rstartree.o: rstartree.cc
	$(CPPC) -c -o rstartree.o rstartree.cc -MMD -MP -MF rstartree.dep
	
util.o: util.cc
	$(CPPC) -c -o util.o util.cc -MMD -MP -MF util.dep

storage.o: storage.cc
	$(CPPC) -c -o storage.o storage.cc -MMD -MP -MF storage.dep


# .cc sources: the %.o rule in makefile.env matches .cpp only, so these
# keep their own rules -- and now write their own dependency files. The
# header prerequisites that were listed by hand named one header each;
# the .dep files name all of them.
-include $(wildcard *.dep)

clean:
	-rm -f *.o *.a *.dep $(LIBDIR)/librstartree.a



