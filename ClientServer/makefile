########################################################################
#
# SECONDO makefile for client server modules 
#
########################################################################

include ../makefile.env

LIBDIROBJECTS := $(LIBDIR)/SecondoInterfaceCS.$(OBJEXT)

APPLICATIONS =\
	$(BINDIR)/SecondoRegistrar$(EXEEXT) \
	$(BINDIR)/SecondoListener$(EXEEXT) \
	$(BINDIR)/SecondoMonitor$(SMIUP)$(EXEEXT) \
	$(BINDIR)/SecondoServer$(SMIUP)$(EXEEXT)

ifneq ($(platform),win32)
DEP_FILES := $(filter-out Win32Socket%, $(DEP_FILES))
else
DEP_FILES := $(filter-out UnixSocket%, $(DEP_FILES))
endif

ifneq ($(smitype),ora)
APPLICATIONS += $(BINDIR)/SecondoCheckpoint$(EXEEXT)
endif

CCFLAGS := $(DEFAULTCCFLAGS) $(JNIINCLUDE)

.PHONY: all
all: $(OBJECTS) $(LIBDIROBJECTS)

.PHONY: buildapp
buildapp: $(APPLICATIONS)

$(LIBDIR)/%.$(OBJEXT): %.$(OBJEXT)
	$(CP) $< $@


# --- Link applications
SERVER_OBJECTS = SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o

$(BINDIR)/SecondoMonitorBDB$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoMonitorBDB$(EXEEXT): SecondoMonitor.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoRegistrar$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoRegistrar$(EXEEXT): SecondoRegistrar.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoCheckpoint$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoCheckpoint$(EXEEXT): SecondoCheckpoint.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoListener$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoListener$(EXEEXT): SecondoListener.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoServerBDB$(EXEEXT): $(ALGLIBFILES) $(SECLIBFILES) 
$(BINDIR)/SecondoServerBDB$(EXEEXT): SecondoServer.o $(SERVER_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(LDJNIFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o,$^) $(LD_LINK_LIBS_ALG)

# Oracle SMI
$(BINDIR)/SecondoMonitorORA$(EXEEXT): $(SECLIBFILES) 
$(BINDIR)/SecondoMonitorORA$(EXEEXT): SecondoMonitor.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $< $(LD_LINK_LIBS)


$(BINDIR)/SecondoServerORA$(EXEEXT): $(ALGLIBFILES) $(SECLIBFILES) 
$(BINDIR)/SecondoServerORA$(EXEEXT): SecondoServer.o $(SERVER_OBJECTS) 
	$(LD) $(EXEFLAGS) $(LDLFAGS) $(LDJNIFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o,$^) $(LD_LINK_LIBS_ALG)


include $(DEP_FILES)

.PHONY: clean
clean:
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBDIROBJECTS) $(APPLICATIONS)

