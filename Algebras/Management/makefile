########################################################################
#
# SECONDO makefile for Algebra and Algebra Manager
#
########################################################################

include ../../makefile.env

OBJECTS :=\
	Algebra.$(OBJEXT) \
	AlgebraManager.$(OBJEXT) \
	AlgebraList.$(OBJEXT)

CCFLAGS = $(PICFLAG) $(DEFAULTCCFLAGS)

CODE_DEPENDENCIES := $(patsubst %.$(OBJEXT), %.dep, $(OBJECTS))


.PHONY: all
all: $(OBJECTS) 

# create dependencies from include directives
AlgebraList.dep: AlgebraList.i

%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)

Algebra.$(OBJEXT): Algebra.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)

ifeq ($(alg),auto)
AlgebraList.i: AlgebraList.i.cfg $(BUILDDIR)/makefile.algebras
	rm -f $@
	@chmod 755 ./alg.bash && ./alg.bash $(ALGEBRAS) $(JNI_ALGEBRAS) > $@
else
AlgebraList.i: AlgebraList.i.cfg
	$(cp-config-file)
endif

AlgebraList.$(OBJEXT): AlgebraList.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)
	cp $@ $(LIBDIR)/$@

AlgebraManager.$(OBJEXT): AlgebraManager.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)

include $(CODE_DEPENDENCIES) 

.PHONY: clean
clean:
	$(RM) $(OBJECTS) *.dep $(LIBDIR)/AlgebraList.$(OBJEXT)

