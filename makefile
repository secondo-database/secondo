########################################################################
#
# SECONDO Makefile
#
########################################################################

# Include platform specification
include makefile.config

# Check whether platform is specified
ifdef platform
include makefile.$(platform)

TOOLOBJECTS=\
	ClientServer/SocketIO.$(OBJEXT) \
	ClientServer/SocketAddress.$(OBJEXT) \
	ClientServer/SocketRuleSet.$(OBJEXT) \
	Tools/NestedLists/NestedList.$(OBJEXT) \
	Tools/NestedLists/NLLex.$(OBJEXT) \
	Tools/NestedLists/NLScanner.$(OBJEXT) \
	Tools/NestedLists/NLParser.$(OBJEXT) \
	Tools/NestedLists/NLParser.tab.$(OBJEXT) \
	Tools/Parser/SecLex.$(OBJEXT) \
	Tools/Parser/SecParser.$(OBJEXT) \
	Tools/Parser/SecParser.tab.$(OBJEXT) \
	Tools/Parser/NestedText.$(OBJEXT) \
	Tools/Utilities/Processes.$(OBJEXT) \
	Tools/Utilities/Application.$(OBJEXT) \
	Tools/Utilities/Messenger.$(OBJEXT) \
	Tools/Utilities/DynamicLibrary.$(OBJEXT) \
	Tools/Utilities/FileSystem.$(OBJEXT) \
	Tools/Utilities/Profiles.$(OBJEXT)

SDBSYSOBJECTS=\
	Algebras/Management/Algebra.$(OBJEXT) \
	Algebras/Management/AlgebraManager.$(OBJEXT) \
	QueryProcessor/QueryProcessor.$(OBJEXT) \
	QueryProcessor/SecondoSystem.$(OBJEXT) \
	QueryProcessor/SecondoCatalog.$(OBJEXT)

ifeq ($(smitype),ora)
SMILIB=$(ORASMILIB)
else
SMILIB=$(BDBSMILIB)
endif

.PHONY: all
all: makedirs buildlibs buildapps

.PHONY: makedirs
makedirs:
	$(MAKE) -C ClientServer
	$(MAKE) -C Tools
	$(MAKE) -C StorageManager
	$(MAKE) -C Algebras
	$(MAKE) -C QueryProcessor
	$(MAKE) -C UserInterfaces

buildlibs: lib/libsdbtool.$(LIBEXT) lib/libsdbsys.$(LIBEXT)
	$(MAKE) -C Algebras buildlibs

# --- Secondo Database Tools library ---

# ... Windows needs special treatment when creating DLLs
ifeq ($(shared),yes)
ifeq ($(platform),win32)
LDOPTTOOL = -Wl,--export-dynamic -Wl,--out-implib,lib/libsdbtool.$(LIBEXT).a
endif
endif

lib/libsdbtool.$(LIBEXT): makedirs $(TOOLOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o lib/libsdbtool.$(LIBEXT) $(LDOPTTOOL) $(TOOLOBJECTS) $(DEFAULTLIB)
ifeq ($(platform),win32)
	$(CP) lib/libsdbtool.$(LIBEXT) bin/libsdbtool.$(LIBEXT)
endif
else
# ... as static library
	$(AR) -r lib/libsdbtool.$(LIBEXT) $(TOOLOBJECTS)
endif

# --- Secondo Database System library ---

# ... Windows needs special treatment when creating DLLs
ifeq ($(shared),yes)
ifeq ($(platform),win32)
LDOPTSYS = -Wl,--export-dynamic -Wl,--out-implib,lib/libsdbsys.$(LIBEXT).a
endif
endif

lib/libsdbsys.$(LIBEXT): makedirs $(SDBSYSOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o lib/libsdbsys.$(LIBEXT) $(LDOPTSYS) $(SDBSYSOBJECTS) $(LIBDIR) $(ALGLIB) $(SMILIB) $(TOOLLIB) $(DEFAULTLIB)
ifeq ($(platform),win32)
	$(CP) lib/libsdbsys.$(LIBEXT) bin/libsdbsys.$(LIBEXT)
endif
else
# ... as static library
	$(AR) -r lib/libsdbsys.$(LIBEXT) $(SDBSYSOBJECTS)
endif

# --- Applications

.PHONY: buildapps
buildapps:
	$(MAKE) -C ClientServer buildapp
	$(MAKE) -C UserInterfaces buildapp

.PHONY: tests
tests: buildlibs
	$(MAKE) -C Tests

#.PHONY: install
#install:
#	$(MAKE) -C lib install

#.PHONY: save
#save:
#	tar cvf SECONDO_`date +%d%m%y`.tar $(LISTSANDTABLESSRC) $(NESTEDLISTSSRC) $(PARSERSRC) $(TUPLEMANAGERSRC) $(ALGEBRAMANAGERSRC) $(QUERYPROCESSORSRC) $(TESTSSRC) $(CLIENTSERVERSRC) $(USERINTERFACESSRC) $(MAKEFILESRC) $(SPECPARSERSRC) $(STORAGEMANAGERSRC) $(ALGEBRAMODULESRC) $(INCLUDEDIR) $(DOCUMENTSDIR) $(FIGURES)

.PHONY: clean
clean:
	$(MAKE) -C ClientServer clean
	$(MAKE) -C StorageManager clean
	$(MAKE) -C Tools clean
	$(MAKE) -C Algebras clean
	$(MAKE) -C QueryProcessor clean
	$(MAKE) -C UserInterfaces clean
	$(RM) *.a
	$(RM) *.so
	$(RM) *.dll

.PHONY: clean_tests
clean_tests:
	$(MAKE) -C Tests clean

.PHONY: clean_all
clean_all: clean clean_tests

.PHONY: distclean
distclean:
	$(MAKE) -C ClientServer distclean
	$(MAKE) -C StorageManager distclean
	$(MAKE) -C Tools distclean
	$(MAKE) -C Algebras distclean
	$(MAKE) -C QueryProcessor distclean
	$(MAKE) -C UserInterfaces distclean

usage:
	@echo *** Usage of the SECONDO makefile:
	@echo ***
	@echo *** make [shared=yes] [target]
	@echo ***
	@echo *** The optional definition shared=yes specifies
	@echo *** that shared libraries are to be built. The
	@echo *** default is to build static libraries.
	@echo ***
	@echo *** The default target is <all>.

else
# Platform NOT specified in make configuration file
usage:
	@echo *** Usage of the SECONDO makefile:
	@echo ***
	@echo *** In the make configuration file makefile.config
	@echo *** exactly one platform definition must be uncommented:
	@echo ***
	@echo *** platform: win32    - Build SECONDO for 32-Bit-Windows
	@echo ***           linux    - Build SECONDO for Linux
	@echo ***           solaris  - Build SECONDO for Solaris
endif

