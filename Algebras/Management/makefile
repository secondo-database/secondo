########################################################################
#
# SECONDO makefile for Algebra and Algebra Manager
#
########################################################################

include ../../makefile.env

CCFLAGS = $(DEFAULTCCFLAGS)

DEP_FILES += AlgebraList.dep

.PHONY: all
all: $(OBJECTS)

AlgebraList.dep: AlgebraList.i

include $(DEP_FILES) 

# special rule for AlgebraList.o
AlgebraList.$(OBJEXT): AlgebraList.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)
	cp $@ $(LIBDIR)/$@

ifeq ($(alg),auto)
AlgebraList.i: AlgebraList.i.cfg $(BUILDDIR)/makefile.algebras
	rm -f $@
	@chmod 755 ./alg.bash && ./alg.bash $(ALGEBRAS) $(JNI_ALGEBRAS) > $@
else
AlgebraList.i: AlgebraList.i.cfg
	$(cp-config-file)
endif

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(DEP_FILES) $(LIBDIR)/AlgebraList.$(OBJEXT)
