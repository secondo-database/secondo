########################################################################
#
# SECONDO Makefile
#
# $Log$
#
########################################################################

include makefile.env

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

buildlibs: $(LIBDIR)/libsdbtool.$(LIBEXT) buildsmi $(LIBDIR)/libsdbsys.$(LIBEXT)
	$(MAKE) -C Algebras buildlibs

buildsmi:
	$(MAKE) -C StorageManager buildlibs

# --- Secondo Database Tools library ---

# ... Windows needs special treatment when creating DLLs
ifeq ($(shared),yes)
ifeq ($(platform),win32)
LDOPTTOOL = -Wl,--export-dynamic -Wl,--out-implib,$(LIBDIR)/libsdbtool.$(LIBEXT).a
endif
endif

$(LIBDIR)/libsdbtool.$(LIBEXT): makedirs $(TOOLOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/libsdbtool.$(LIBEXT) $(LDOPTTOOL) $(TOOLOBJECTS) $(DEFAULTLIB)
ifeq ($(platform),win32)
	$(CP) $(LIBDIR)/libsdbtool.$(LIBEXT) $(BINDIR)/libsdbtool.$(LIBEXT)
endif
else
# ... as static library
	$(AR) -r $(LIBDIR)/libsdbtool.$(LIBEXT) $(TOOLOBJECTS)
endif

# --- Secondo Database System library ---

# ... Windows needs special treatment when creating DLLs
ifeq ($(shared),yes)
ifeq ($(platform),win32)
LDOPTSYS = -Wl,--export-dynamic -Wl,--out-implib,$(LIBDIR)/libsdbsys.$(LIBEXT).a
endif
endif

$(LIBDIR)/libsdbsys.$(LIBEXT): makedirs $(SDBSYSOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/libsdbsys.$(LIBEXT) $(LDOPTSYS) $(SDBSYSOBJECTS) -L$(LIBDIR) $(SMILIB) $(TOOLLIB) $(DEFAULTLIB)
ifeq ($(platform),win32)
	$(CP) $(LIBDIR)/libsdbsys.$(LIBEXT) $(BINDIR)/libsdbsys.$(LIBEXT)
endif
else
# ... as static library
	$(AR) -r $(LIBDIR)/libsdbsys.$(LIBEXT) $(SDBSYSOBJECTS)
endif

# --- Applications

.PHONY: buildapps
buildapps:
	$(MAKE) -C ClientServer buildapp
	$(MAKE) -C UserInterfaces buildapp

.PHONY: tests
tests: buildlibs
	$(MAKE) -C Tests


# rules for deployment of source files
# default value for cvs tag 
ifndef tag
tag=HEAD
endif

.PHONY: save_sources
save_sources: secondo.tgz

secondo.tgz:
	cvs export -r$(tag) secondo
	tar -cvf secondo.tar secondo/*
	rm -r secondo
	mv secondo.tar secondo
	gzip -S .tgz secondo


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
	$(RM) $(LIBDIR)/libsdb*.a $(LIBDIR)/libsdb*.so $(LIBDIR)/libsdb*.dll $(LIBDIR)/libsdb*.dll.a
	$(RM) $(LIBDIR)/libsmi*.a $(LIBDIR)/libsmi*.so $(LIBDIR)/libsmi*.dll $(LIBDIR)/libsmi*.dll.a
	$(RM) $(LIBDIR)/SecondoInterface*.o $(LIBDIR)/SecondoInterface*.lo
	$(RM) $(LIBDIR)/AlgebraList.o $(LIBDIR)/AlgebraList.lo

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

.PHONY: usage
usage:
	@echo "*** Usage of the SECONDO makefile:"
	@echo "***"
	@echo "*** make [shared=yes] [target]"
	@echo "***"
	@echo "*** The optional definition shared=yes specifies"
	@echo "*** that shared libraries are to be built. The"
	@echo "*** default is to build static libraries."
	@echo "***"
	@echo "*** The default target is <all>."

