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

.PHONY: all
all: $(OBJECTS)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

ifeq ($(platform),win32)
SOCKETIMPL=Win32Socket.cpp
else
SOCKETIMPL=UnixSocket.cpp
endif

SocketIO.$(OBJEXT): SocketIO.cpp ../include/SocketIO.h $(SOCKETIMPL)
	$(CC) -c -o SocketIO.$(OBJEXT) SocketIO.cpp $(CCFLAGS)

SocketAddress.$(OBJEXT): SocketAddress.cpp ../include/SocketIO.h
	$(CC) -c -o SocketAddress.$(OBJEXT) SocketAddress.cpp $(CCFLAGS)

SocketRuleSet.$(OBJEXT): SocketRuleSet.cpp ../include/SocketIO.h
	$(CC) -c -o SocketRuleSet.$(OBJEXT) SocketRuleSet.cpp $(CCFLAGS)

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

