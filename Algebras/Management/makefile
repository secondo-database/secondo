########################################################################
#
# SECONDO makefile for Algebra and Algebra Manager
#
########################################################################

include ../../makefile.env

OBJECTS=\
	Algebra.$(OBJEXT) \
	AlgebraManager.$(OBJEXT) \
	$(LIBDIR)/AlgebraList.$(OBJEXT)

CCFLAGS = $(PICFLAG) $(DEFAULTCCFLAGS)

CODE_DEPENDENCIES := $(patsubst %.$(OBJEXT), %.dep, $(OBJECTS))

.PHONY: all
all: $(OBJECTS) 

# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

$(LIBDIR)/%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)

Algebra.$(OBJEXT): Algebra.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)

$(LIBDIR)/AlgebraList.$(OBJEXT): AlgebraList.cpp AlgebraList.i
	$(CC) -c -o $@ $< $(CCFLAGS)

AlgebraManager.$(OBJEXT): AlgebraManager.cpp AlgebraList.i
	$(CC) -c -o $@ $< $(CCFLAGS)

include $(CODE_DEPENDENCIES:.cpp=.dep)

.PHONY: clean
clean:
	$(RM) $(OBJECTS)

