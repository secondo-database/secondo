########################################################################
#
# SECONDO makefile for query processor, catalog, system and interface
#
########################################################################

include ../makefile.env

OBJECTS=\
	QueryProcessor.$(OBJEXT) \
	SecondoCatalog.$(OBJEXT) \
	SecondoSystem.$(OBJEXT) \
	SecondoInterface.$(OBJEXT) \
	SecondoInterfaceGeneral.$(OBJEXT)

LIBOBJECTS=\
	$(LIBDIR)/SecondoInterface.$(OBJEXT) \
	$(LIBDIR)/SecondoInterfaceGeneral.$(OBJEXT)

CODE_DEPENDENCIES = $(patsubst %.$(OBJEXT), %.dep, $(OBJECTS))

.PHONY: all
all: $(CODE_DEPENDENCIES) $(OBJECTS) $(LIBOBJECTS)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS) -I../Algebras/Relation-C++

# create dependencies from include directives
%.dep: %.cpp 
	$(CC) -MM $(CCFLAGS) $< > $@

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS) 

$(LIBDIR)/%.$(OBJEXT): %.$(OBJEXT)
	$(CP) $< $@ 

include $(CODE_DEPENDENCIES:.cpp=.dep)

.PHONY: clean
clean: 
	$(RM) *.dep $(OBJECTS) $(LIBOBJECTS)


