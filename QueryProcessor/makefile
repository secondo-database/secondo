########################################################################
#
# SECONDO makefile for query processor, catalog, system and interface
#
########################################################################

include ../makefile.env

LIBOBJECTS=\
	$(LIBDIR)/SecondoInterface.$(OBJEXT) \
	$(LIBDIR)/SecondoInterfaceGeneral.$(OBJEXT)

.PHONY: all
all: $(OBJECTS) $(LIBOBJECTS)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS) -I../Algebras/Relation-C++

$(LIBDIR)/%.$(OBJEXT): %.$(OBJEXT)
	$(CP) $< $@ 

include $(DEP_FILES)

.PHONY: clean
clean: 
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBOBJECTS)


