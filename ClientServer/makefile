########################################################################
#
# SECONDO makefile for client server modules 
#
########################################################################

include ../makefile.env

OBJECTS=\
	SocketIO \
	SocketAddress \
	SocketRuleSet

APPLOBJECTS=\
	SecondoMonitor \
	SecondoRegistrar \
	SecondoCheckpoint \
	SecondoListener \
	SecondoServer

APPLOBJECTS += $(LIBDIR)/SecondoInterfaceCS

ALL_OBJECTS = $(addsuffix .$(OBJEXT), $(OBJECTS) $(APPLOBJECTS))
CODE_DEPENDENCIES = $(patsubst %.$(OBJEXT), %.dep, $(ALL_OBJECTS))

APPLICATIONS =\
	$(BINDIR)/SecondoRegistrar$(EXEEXT) \
	$(BINDIR)/SecondoListener$(EXEEXT) \
	$(BINDIR)/SecondoMonitor$(SMIUP)$(EXEEXT) \
	$(BINDIR)/SecondoServer$(SMIUP)$(EXEEXT)

ifneq ($(smitype),ora)
APPLICATIONS += $(BINDIR)/SecondoCheckpoint$(EXEEXT)
endif

CCFLAGS := $(DEFAULTCCFLAGS)

.PHONY: all
all: $(ALL_OBJECTS) 

.PHONY: buildapp
buildapp: $(APPLICATIONS)
	echo $(APPLICATIONS) 

# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

$(LIBDIR)/%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(BDBINCLUDE) $(CCFLAGS) $(JNIINCLUDE)

$(LIBDIR)/%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(BDBINCLUDE) $(CCFLAGS) $(JNIINCLUDE)


# --- Link applications
SERVER_OBJECTS = SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o

$(BINDIR)/SecondoMonitorBDB$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoMonitorBDB$(EXEEXT): SecondoMonitor.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoRegistrar$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoRegistrar$(EXEEXT): SecondoRegistrar.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoCheckpoint$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoCheckpoint$(EXEEXT): SecondoCheckpoint.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoListener$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoListener$(EXEEXT): SecondoListener.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoServerBDB$(EXEEXT): $(ALGLIBFILES) $(SECLIBFILES) 
$(BINDIR)/SecondoServerBDB$(EXEEXT): SecondoServer.o $(SERVER_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(LDJNIFLAGS) -o $@ $(filter %.o,$^) $(LD_LINK_LIBS_ALG)

# Oracle SMI
$(BINDIR)/SecondoMonitorORA$(EXEEXT): $(SECLIBFILES) 
$(BINDIR)/SecondoMonitorORA$(EXEEXT): SecondoMonitor.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $< $(LD_LINK_LIBS)


$(BINDIR)/SecondoServerORA$(EXEEXT): $(ALGLIBFILES) $(SECLIBFILES) 
$(BINDIR)/SecondoServerORA$(EXEEXT): SecondoServer.o $(SERVER_OBJECTS) 
	$(LD) $(EXEFLAGS) $(LDLFAGS) $(LDJNIFLAGS) -o $@ $(filter %.o,$^) $(LD_LINK_LIBS_ALG)


include $(CODE_DEPENDENCIES:.cpp=.dep)

.PHONY: clean
clean:
	$(RM) *.dep $(ALL_OBJECTS) $(APPLICATIONS)

