########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../makefile.env
include ../makefile.jni

OBJECTS=\
	SocketIO.$(OBJEXT) \
	SocketAddress.$(OBJEXT) \
	SocketRuleSet.$(OBJEXT)

APPLOBJECTS=\
	SecondoMonitor.o \
	SecondoRegistrar.o \
	SecondoCheckpoint.o \
	SecondoListener.o \
	$(LIBDIR)/SecondoInterfaceCS.o \
	SecondoServer.o

APPLICATIONS1=\
	$(BINDIR)/SecondoRegistrar$(EXEEXT) \
	$(BINDIR)/SecondoListener$(EXEEXT)

ifeq ($(smitype),ora)
APPLICATIONS = $(APPLICATIONS1) $(BINDIR)/SecondoMonitorORA$(EXEEXT) $(BINDIR)/SecondoServerORA$(EXEEXT)
else
APPLICATIONS = $(APPLICATIONS1) $(BINDIR)/SecondoMonitorBDB$(EXEEXT) $(BINDIR)/SecondoServerBDB$(EXEEXT) $(BINDIR)/SecondoCheckpoint$(EXEEXT)
endif

.PHONY: all
all: $(APPLOBJECTS)

.PHONY: socket
socket: $(OBJECTS)

.PHONY: buildapp
buildapp: $(APPLICATIONS)

CCFLAGS=  $(PICFLAG) $(DEFAULTCCFLAGS)
CCXFLAGS= $(DEFAULTCCFLAGS)

ifeq ($(platform),win32)
SOCKETIMPL=Win32Socket.cpp
else
SOCKETIMPL=UnixSocket.cpp
endif

SocketIO.$(OBJEXT): SocketIO.cpp SecondoConfig.h SocketIO.h $(SOCKETIMPL)
	$(CC) -c -o SocketIO.$(OBJEXT) SocketIO.cpp $(CCFLAGS)

SocketAddress.$(OBJEXT): SocketAddress.cpp SecondoConfig.h SocketIO.h
	$(CC) -c -o SocketAddress.$(OBJEXT) SocketAddress.cpp $(CCFLAGS)

SocketRuleSet.$(OBJEXT): SocketRuleSet.cpp SecondoConfig.h SocketIO.h
	$(CC) -c -o SocketRuleSet.$(OBJEXT) SocketRuleSet.cpp $(CCFLAGS)

# --- Client/Server Applications

SecondoMonitor.o: SecondoMonitor.cpp Application.h  Processes.h SecondoSMI.h Profiles.h FileSystem.h
	$(CC) -c -o $@ $< $(CCXFLAGS)

SecondoRegistrar.o: SecondoRegistrar.cpp Application.h Processes.h SocketIO.h Profiles.h
	$(CC) -c -o $@ $< $(CCXFLAGS)

SecondoCheckpoint.o: SecondoCheckpoint.cpp Application.h Processes.h SocketIO.h Profiles.h
	$(CC) -c -o $@ $< $(BDBINCLUDE) $(CCXFLAGS)

SecondoListener.o: SecondoListener.cpp Application.h Processes.h SocketIO.h Profiles.h
	$(CC) -c -o $@ $< $(CCXFLAGS)

SecondoServer.o: SecondoServer.cpp Application.h SocketIO.h Profiles.h AlgebraTypes.h SecondoSystem.h SecondoCatalog.h SecondoInterface.h Messenger.h Profiles.h
	$(CC) -c -o $@ $< $(CCXFLAGS) $(JNIINCLUDE)

$(LIBDIR)/SecondoInterfaceCS.o: SecondoInterfaceCS.cpp SecondoInterface.h Profiles.h
	$(CC) -c -o $@ $< $(CCXFLAGS)

# --- Link applications

$(BINDIR)/SecondoMonitorBDB$(EXEEXT): SecondoMonitor.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $< -L$(LIBDIR) $(BDBSMILIB) $(TOOLLIB) $(BDBLIBDIR) $(BDBLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoMonitorORA$(EXEEXT): SecondoMonitor.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $< -L$(LIBDIR) $(ORASMILIB) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoRegistrar$(EXEEXT): SecondoRegistrar.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $< -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoCheckpoint$(EXEEXT): SecondoCheckpoint.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $< -L$(LIBDIR) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoListener$(EXEEXT): SecondoListener.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $< -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoServerBDB$(EXEEXT): SecondoServer.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o $(TUPLEMANAGER) libsdbtool.$(LIBEXT)  
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $(filter %.o,$^) -L$(LIBDIR) $(SDBLIB) $(BDBSMILIB) $(ALGLIB) $(TOOLLIB) $(BDBLIBDIR) $(BDBLIB) $(DEFAULTLIB) $(JNILINKOPTS)

$(BINDIR)/SecondoServerORA$(EXEEXT): SecondoServer.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) $(LDLFAGS) -o $@ $(filter %.o,$^) -L$(LIBDIR) $(SDBLIB) $(ORASMILIB) $(ALGLIB) $(TOOLLIB) $(DEFAULTLIB)

include ../include.dep

.PHONY: clean
clean:
	$(RM) $(APPLOBJECTS) $(APPLICATIONS)

.PHONY: clean_socket
clean_socket:
	$(RM) $(OBJECTS) 

