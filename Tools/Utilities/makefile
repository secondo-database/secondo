########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../../makefile.config
ifdef platform
include ../../makefile.$(platform)
.SUFFIXES: .exe .res .a .o .c .cpp .cc .cxx .m .rc .p .f .F .r .y .l .s .S .def .h

OBJECTS=\
	Processes.$(OBJEXT) \
	Application.$(OBJEXT) \
	Messenger.$(OBJEXT) \
	DynamicLibrary.$(OBJEXT) \
	Profiles.$(OBJEXT) \
	FileSystem.$(OBJEXT)

.PHONY: all
all: $(OBJECTS)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

DynamicLibrary.$(OBJEXT): DynamicLibrary.cpp ../../include/DynamicLibrary.h
	$(CC) -c -o DynamicLibrary.$(OBJEXT) DynamicLibrary.cpp $(CCFLAGS)

Profiles.$(OBJEXT): Profiles.cpp ../../include/Profiles.h
	$(CC) -c -o Profiles.$(OBJEXT) Profiles.cpp $(CCFLAGS)

FileSystem.$(OBJEXT): FileSystem.cpp ../../include/FileSystem.h
	$(CC) -c -o FileSystem.$(OBJEXT) FileSystem.cpp $(CCFLAGS)

Processes.$(OBJEXT): Processes.cpp ../../include/Processes.h ../../include/SocketIO.h ../../include/SecondoConfig.h
	$(CC) -c -o Processes.$(OBJEXT) Processes.cpp $(CCFLAGS)

Application.$(OBJEXT): Application.cpp ../../include/Application.h ../../include/SocketIO.h ../../include/SecondoConfig.h
	$(CC) -c -o Application.$(OBJEXT) Application.cpp $(CCFLAGS)

Messenger.$(OBJEXT): Messenger.cpp ../../include/Messenger.h
	$(CC) -c -o Messenger.$(OBJEXT) Messenger.cpp $(CCFLAGS)

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

