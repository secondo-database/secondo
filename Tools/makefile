########################################################################
#
# SECONDO makefile for tool and utility modules
#
########################################################################

include ../makefile.config
ifdef platform
include ../makefile.$(platform)

OBJECTS=\
	../ClientServer/SocketIO.$(OBJEXT) \
	../ClientServer/SocketAddress.$(OBJEXT) \
	../ClientServer/SocketRuleSet.$(OBJEXT) \
	NestedLists/NestedList.$(OBJEXT) \
	NestedLists/NLLex.$(OBJEXT) \
	NestedLists/NLScanner.$(OBJEXT) \
	NestedLists/NLParser.$(OBJEXT) \
	NestedLists/NLParser.tab.$(OBJEXT) \
	Parser/SecLex.$(OBJEXT) \
	Parser/SecParser.$(OBJEXT) \
	Parser/SecParser.tab.$(OBJEXT) \
	Parser/NestedText.$(OBJEXT) \
	Utilities/Processes.$(OBJEXT) \
	Utilities/Application.$(OBJEXT) \
	Utilities/Messenger.$(OBJEXT) \
	Utilities/DynamicLibrary.$(OBJEXT) \
	Utilities/FileSystem.$(OBJEXT) \
	Utilities/Profiles.$(OBJEXT)

.PHONY: all
all: libsdbtool.$(LIBEXT)

.PHONY: makedirs
makedirs:
	$(MAKE) -C NestedLists
	$(MAKE) -C Parser
	$(MAKE) -C Utilities

# --- Storage Management Interface library ---

# ... Windows needs special treatment when creating DLLs
ifeq ($(shared),yes)
ifeq ($(platform),win32)
LDOPT = -Wl,--export-dynamic -Wl,--out-implib,libsdbtool.$(LIBEXT).a
endif
endif

libsdbtool.$(LIBEXT): makedirs
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o libsdbtool.$(LIBEXT) $(LDOPT) $(OBJECTS) $(DEFAULTLIB)
	$(CP) libsdbtool.$(LIBEXT) ../lib/libsdbtool.$(LIBEXT)
ifeq ($(platform),win32)
	$(CP) libsdbtool.$(LIBEXT) ../bin/libsdbtool.$(LIBEXT)
	$(CP) libsdbtool.$(LIBEXT).a ../lib/libsdbtool.$(LIBEXT).a
endif
else
# ... as static library
	$(AR) -r libsdbtool.$(LIBEXT) $(OBJECTS)
	$(CP) libsdbtool.$(LIBEXT) ../lib/libsdbtool.$(LIBEXT)
endif

# --- Storage Management Interface modules ---

.PHONY: clean
clean:
	$(MAKE) -C NestedLists clean
	$(MAKE) -C Parser clean
	$(MAKE) -C Utilities clean
	$(RM) *.a
	$(RM) *.so
	$(RM) *.dll

else
# Platform not specified
.DEFAULT:
	@echo *** Platform not specified in makefile.config
endif

