########################################################################
#
# SECONDO makefile for lists and tables
#
########################################################################

include ../../makefile.config
ifdef platform
include ../../makefile.$(platform)
.SUFFIXES: .exe .res .a .o .c .cpp .cc .cxx .m .rc .p .f .F .r .y .l .s .S .def .h

#OBJECTS=\
#	filesystem.$(OBJEXT)

.PHONY: all
all: $(OBJECTS)
	$(CP) CTable.h ../../include
	$(CP) CTable.cpp ../../include

#CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

#profiles.$(OBJEXT): profiles.cpp ../../include/profiles.h
#	$(CC) -c -o profiles.$(OBJEXT) profiles.cpp $(CCFLAGS)

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

