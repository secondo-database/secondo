########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../../makefile.env

CCFLAGS = $(PICFLAG) $(DEFAULTCCFLAGS)

.PHONY: all
all:  $(OBJECTS)

include $(DEP_FILES)

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(DEP_FILES) 

