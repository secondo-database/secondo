########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../../makefile.env

OBJECTS=\
	Processes.$(OBJEXT) \
	Application.$(OBJEXT) \
	Messenger.$(OBJEXT) \
	DynamicLibrary.$(OBJEXT) \
	Profiles.$(OBJEXT) \
	FileSystem.$(OBJEXT) \
	UtilFunctions.$(OBJEXT)

VPATH = $(INCLUDEDIR)

.PHONY: all
all: $(OBJECTS)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

DynamicLibrary.$(OBJEXT): DynamicLibrary.cpp DynamicLibrary.h SecondoConfig.h
	$(CC) -c -o $@ $< $(CCFLAGS)

Profiles.$(OBJEXT): Profiles.cpp Profiles.h
	$(CC) -c -o $@ $< $(CCFLAGS)

FileSystem.$(OBJEXT): FileSystem.cpp FileSystem.h SecondoConfig.h
	$(CC) -c -o $@ $< $(CCFLAGS)

Processes.$(OBJEXT): Processes.cpp Processes.h
	$(CC) -c -o $@ $< $(CCFLAGS)

Application.$(OBJEXT): Application.cpp Application.h SecondoConfig.h
	$(CC) -c -o $@ $< $(CCFLAGS)

Messenger.$(OBJEXT): Messenger.cpp Messenger.h SocketIO.h
	$(CC) -c -o $@ $< $(CCFLAGS)

UtilFunctions.$(OBJEXT): UtilFunctions.cpp TimeTest.h
	$(CC) -c -o $@ $< $(CCFLAGS)


include ../../include.dep

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo

