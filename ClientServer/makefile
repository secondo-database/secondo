########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../makefile.config
ifdef platform
include ../makefile.$(platform)
.SUFFIXES: .exe .res .a .o .c .cpp .cc .cxx .m .rc .p .f .F .r .y .l .s .S .def .h

OBJECTS=\
	SocketIO.$(OBJEXT) \
	SocketAddress.$(OBJEXT) \
	SocketRuleSet.$(OBJEXT)

APPLOBJECTS=\
	SecondoMonitor.o \
	SecondoRegistrar.o \
	SecondoCheckpoint.o \
	SecondoListener.o \
	SecondoInterfaceCS.o \
	SecondoServer.o

APPLICATIONS=\
	../bin/SecondoRegistrar$(EXEEXT) \
	../bin/SecondoListener$(EXEEXT)

.PHONY: all
all: $(OBJECTS) $(APPLOBJECTS)

.PHONY: buildapp
ifeq ($(smitype),ora)
buildapp: $(APPLICATIONS) ../bin/SecondoMonitorORA$(EXEEXT) ../bin/SecondoServerORA$(EXEEXT)
else
buildapp: $(APPLICATIONS) ../bin/SecondoMonitorBDB$(EXEEXT) ../bin/SecondoServerBDB$(EXEEXT) ../bin/SecondoCheckpoint$(EXEEXT)
endif

CCFLAGS=  $(PICFLAG) $(DEFAULTCCFLAGS)
CCXFLAGS= $(DEFAULTCCFLAGS)

ifeq ($(platform),win32)
SOCKETIMPL=Win32Socket.cpp
else
SOCKETIMPL=UnixSocket.cpp
endif

SocketIO.$(OBJEXT): SocketIO.cpp $(INCLUDEDIR)/SecondoConfig.h $(INCLUDEDIR)/SocketIO.h $(SOCKETIMPL)
	$(CC) -c -o SocketIO.$(OBJEXT) SocketIO.cpp $(CCFLAGS)

SocketAddress.$(OBJEXT): SocketAddress.cpp $(INCLUDEDIR)/SecondoConfig.h $(INCLUDEDIR)/SocketIO.h
	$(CC) -c -o SocketAddress.$(OBJEXT) SocketAddress.cpp $(CCFLAGS)

SocketRuleSet.$(OBJEXT): SocketRuleSet.cpp $(INCLUDEDIR)/SecondoConfig.h $(INCLUDEDIR)/SocketIO.h
	$(CC) -c -o SocketRuleSet.$(OBJEXT) SocketRuleSet.cpp $(CCFLAGS)

# --- Client/Server Applications

SecondoMonitor.o: SecondoMonitor.cpp $(INCLUDEDIR)/Application.h  $(INCLUDEDIR)/Processes.h $(INCLUDEDIR)/SecondoSMI.h $(INCLUDEDIR)/Profiles.h $(INCLUDEDIR)/FileSystem.h
	$(CC) -c -o SecondoMonitor.o SecondoMonitor.cpp $(CCXFLAGS)

SecondoRegistrar.o: SecondoRegistrar.cpp $(INCLUDEDIR)/Application.h  $(INCLUDEDIR)/Processes.h $(INCLUDEDIR)/SocketIO.h $(INCLUDEDIR)/Profiles.h
	$(CC) -c -o SecondoRegistrar.o SecondoRegistrar.cpp $(CCXFLAGS)

SecondoCheckpoint.o: SecondoCheckpoint.cpp $(INCLUDEDIR)/Application.h $(INCLUDEDIR)/Processes.h $(INCLUDEDIR)/SocketIO.h $(INCLUDEDIR)/Profiles.h
	$(CC) -c -o SecondoCheckpoint.o SecondoCheckpoint.cpp $(BDBINCLUDE) $(CCXFLAGS)

SecondoListener.o: SecondoListener.cpp $(INCLUDEDIR)/Application.h $(INCLUDEDIR)/Processes.h $(INCLUDEDIR)/SocketIO.h $(INCLUDEDIR)/Profiles.h
	$(CC) -c -o SecondoListener.o SecondoListener.cpp $(CCXFLAGS)

SecondoServer.o: SecondoServer.cpp $(INCLUDEDIR)/Application.h $(INCLUDEDIR)/SocketIO.h $(INCLUDEDIR)/Profiles.h $(INCLUDEDIR)/AlgebraTypes.h $(INCLUDEDIR)/SecondoSystem.h $(INCLUDEDIR)/SecondoCatalog.h $(INCLUDEDIR)/SecondoInterface.h
	$(CC) -c -o SecondoServer.o SecondoServer.cpp $(CCXFLAGS)

SecondoInterfaceCS.o: SecondoInterfaceCS.cpp $(INCLUDEDIR)/SecondoInterface.h $(INCLUDEDIR)/Profiles.h
	$(CC) -c -o SecondoInterfaceCS.o SecondoInterfaceCS.cpp $(CCXFLAGS)
	$(CP) SecondoInterfaceCS.$(OBJEXT) ../lib/SecondoInterfaceCS.$(OBJEXT)

# --- Link applications

../bin/SecondoMonitorBDB$(EXEEXT): SecondoMonitor.o ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o ../bin/SecondoMonitorBDB$(EXEEXT) SecondoMonitor.o $(LIBDIR) $(BDBSMILIB) $(TOOLLIB) $(BDBLIBDIR) $(BDBLIB) $(DEFAULTLIB)

../bin/SecondoMonitorORA$(EXEEXT): SecondoMonitor.o ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o ../bin/SecondoMonitorORA$(EXEEXT) SecondoMonitor.o $(LIBDIR) $(ORASMILIB) $(TOOLLIB) $(DEFAULTLIB)

../bin/SecondoRegistrar$(EXEEXT): SecondoRegistrar.o ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o ../bin/SecondoRegistrar$(EXEEXT) SecondoRegistrar.o  $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

../bin/SecondoCheckpoint$(EXEEXT): SecondoCheckpoint.o ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o ../bin/SecondoCheckpoint$(EXEEXT) SecondoCheckpoint.o $(LIBDIR) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)

../bin/SecondoListener$(EXEEXT): SecondoListener.o ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o ../bin/SecondoListener$(EXEEXT) SecondoListener.o $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

../bin/SecondoServerBDB$(EXEEXT): SecondoServer.o ../lib/SecondoInterface.$(OBJEXT) ../lib/SecondoInterfaceGeneral.$(OBJEXT) ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o ../bin/SecondoServerBDB$(EXEEXT) SecondoServer.o ../lib/SecondoInterface.$(OBJEXT) ../lib/SecondoInterfaceGeneral.$(OBJEXT) $(LIBDIR) $(SDBLIB) $(BDBSMILIB) $(ALGLIB) $(TOOLLIB) $(BDBLIBDIR) $(BDBLIB) $(DEFAULTLIB)

../bin/SecondoServerORA$(EXEEXT): SecondoServer.o ../lib/SecondoInterface.$(OBJEXT) ../lib/SecondoInterfaceGeneral.$(OBJEXT) ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o ../bin/SecondoServerORA$(EXEEXT) SecondoServer.o ../lib/SecondoInterface.$(OBJEXT) ../lib/SecondoInterfaceGeneral.$(OBJEXT) $(LIBDIR) $(SDBLIB) $(ORASMILIB) $(ALGLIB) $(TOOLLIB) $(DEFAULTLIB)

include ../include.dep

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

