########################################################################
#
# SECONDO Makefile
#
# $Log$
# Revision 1.19  2003/07/25 13:00:24  behr
# add the target javagui
#
# Revision 1.18  2003/06/27 14:01:52  behr
# jni support added
#
# Revision 1.17  2003/04/17 21:04:16  telle
# Fixed makefile problems when building shared library version. (LIBNAME needs to be a target specific variable if there are more than one library to be built within one makefile.)
# Definition of variable SMILIB moved from main makefile to makefile.env.
# Changed references to variable BDBSMILIB to SMILIB in several algebra modul makefiles to make them independent of the used SMI implementation.
#
# Revision 1.16  2003/04/09 12:49:53  spieker
# Rule dist modified.
#
# Revision 1.15  2003/04/09 12:19:53  spieker
# New target dist creates tar.gz files of the secondo source code
#
# Revision 1.14  2003/01/24 11:56:07  spieker
# Some modifications on setting environments. If the prolog related
# environment variables are set SecondoPL will be compiled automatically.
#
# Revision 1.13  2003/01/23 18:12:09  spieker
# Makefiles revised for compiling the secondoPL userinterface.
#
# Revision 1.12  2003/01/22 21:32:55  spieker
# Problems with shared=yes. Variable LDOPTOOL was not defined.
#
# Revision 1.11  2002/11/27 20:31:04  spieker
# Rules for generation of libraries were revised. Many rules are written
# in a short manner by use of VPATH and automatic variables $@ and $<.
# Hopefully the makefiles are now easier to read and to maintain.
#
# Revision 1.10  2002/11/26 16:12:16  spieker
# Dependency of static libraries simplified.
#
# Revision 1.9  2002/11/26 09:31:23  spieker
# new object UtilFunctions in tools library added
#
# Revision 1.8  2002/09/26 17:11:32  spieker
# New rule save_sources which creates a gzipped tar file of the sources. The CVS keyword Log was added in the file header.
#
#
########################################################################

include makefile.env

TOOLOBJECTS_BASENAMES=\
	Tools/NestedLists/NestedList \
	Tools/NestedLists/NLLex \
	Tools/NestedLists/NLScanner \
	Tools/NestedLists/NLParser \
	Tools/NestedLists/NLParser.tab \
	Tools/Parser/SecLex \
	Tools/Parser/SecParser \
	Tools/Parser/SecParser.tab \
	Tools/Parser/NestedText 


UTILOBJECTS_BASENAMES=\
	Tools/Utilities/Processes \
	Tools/Utilities/Application \
	Tools/Utilities/Messenger \
	Tools/Utilities/DynamicLibrary \
	Tools/Utilities/FileSystem \
	Tools/Utilities/Profiles \
	Tools/Utilities/UtilFunctions \
	ClientServer/SocketIO \
	ClientServer/SocketAddress \
	ClientServer/SocketRuleSet



SDBSYSOBJECTS_BASENAMES=\
	Algebras/Management/Algebra \
	Algebras/Management/AlgebraManager \
	QueryProcessor/QueryProcessor \
	QueryProcessor/SecondoSystem \
	QueryProcessor/SecondoCatalog


TOOLOBJECTS = $(addsuffix .$(OBJEXT), $(TOOLOBJECTS_BASENAMES))
UTILOBJECTS = $(addsuffix .$(OBJEXT), $(UTILOBJECTS_BASENAMES))
SDBSYSOBJECTS = $(addsuffix .$(OBJEXT), $(SDBSYSOBJECTS_BASENAMES))

ifeq ($(smitype),ora)
SMILIB=$(ORASMILIB)
else
SMILIB=$(BDBSMILIB)
endif

.PHONY: all
all: showjni makedirs buildlibs buildalg buildapps showjavagui

.PHONY: showjni
showjni:
	@echo $(JNITEXT)
	
.PHONY: showjavagui
showjavagui:
	@echo "if you want to use the java based gui"
	@echo "enter make javagui"

.PHONY: javagui
javagui:
	$(MAKE) -C Javagui all	

.PHONY: clientserver
clientserver:
	$(MAKE) -C ClientServer
	$(MAKE) -C UserInterfaces client
	$(MAKE) -C ClientServer buildapp

.PHONY: makedirs
makedirs:
	$(MAKE) -C ClientServer socket
	$(MAKE) -C Tools
	$(MAKE) -C StorageManager
	$(MAKE) -C Algebras/Management
	$(MAKE) -C QueryProcessor
	$(MAKE) -C UserInterfaces

.PHONY: buildlibs
buildlibs: $(LIBDIR)/libsdbtool.$(LIBEXT) $(LIBDIR)/libsdbutils.$(LIBEXT) buildsmi $(LIBDIR)/libsdbsys.$(LIBEXT)
	$(MAKE) -C Algebras buildlibs

.PHONY: buildsmi
buildsmi:
	$(MAKE) -C StorageManager buildlibs

.PHONY: buildalg
buildalg:
	$(MAKE) -C Algebras


# --- Secondo Database Tools library ---

# Windows needs special treatment for dynamic libraries!
# The variable LIBNAME will be used in the LDOPTTOOL variable 
# which is only defined makefile.win32 included by makefile.env.

$(LIBDIR)/libsdbtool.$(LIBEXT): LIBNAME=libsdbtool
$(LIBDIR)/libsdbtool.$(LIBEXT): $(TOOLOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/libsdbtool.$(LIBEXT) $(LDOPT) $(TOOLOBJECTS) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/libsdbtool.$(LIBEXT) $(TOOLOBJECTS)
endif

# --- Secondo Database util library ---

LIBNAME=libsdbutils
$(LIBDIR)/libsdbutils.$(LIBEXT): $(UTILOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/libsdbutils.$(LIBEXT) $(LDOPTTOOL) $(UTILOBJECTS) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/libsdbutils.$(LIBEXT) $(UTILOBJECTS)
endif

# --- Secondo Database System library ---

$(LIBDIR)/libsdbsys.$(LIBEXT): LIBNAME=libsdbsys
$(LIBDIR)/libsdbsys.$(LIBEXT): $(SDBSYSOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/libsdbsys.$(LIBEXT) $(LDOPT) $(SDBSYSOBJECTS) -L$(LIBDIR) $(BDBSMILIB) $(SMILIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/libsdbsys.$(LIBEXT) $(SDBSYSOBJECTS)
endif
	

# --- Applications

.PHONY: buildapps
buildapps: 
	$(MAKE) -C UserInterfaces buildapp

.PHONY: tests
tests: buildlibs
	$(MAKE) -C Tests


# rules for deployment of source files
# default value for cvs tag 
ifndef tag
tag=HEAD
endif

ifeq ($(platform),win32)
  NETDEV = /cvs-projects/SECONDO_CD/Windows
else
  NETDEV = /cvs-projects/SECONDO_CD/Unix
endif

.PHONY: dist
dist:	secondo.tgz


secondo.tgz:
	cvs export -r$(tag) secondo
	tar -czf  secondo.tgz secondo/*
	cp secondo.tgz $(NETDEV)
	rm -r secondo


.PHONY: clean
clean:
	$(MAKE) -C ClientServer clean_socket
	$(MAKE) -C StorageManager clean
	$(MAKE) -C Tools clean
	$(MAKE) -C Algebras clean
	$(MAKE) -C QueryProcessor clean
	$(MAKE) -C UserInterfaces clean
	$(RM) $(LIBDIR)/libsdb*.a $(LIBDIR)/libsdb*.so $(LIBDIR)/libsdb*.dll $(LIBDIR)/libsdb*.dll.a
	$(RM) $(LIBDIR)/libsmi*.a $(LIBDIR)/libsmi*.so $(LIBDIR)/libsmi*.dll $(LIBDIR)/libsmi*.dll.a
	$(RM) $(LIBDIR)/SecondoInterface*.o $(LIBDIR)/SecondoInterface*.lo
	$(RM) $(LIBDIR)/AlgebraList.o $(LIBDIR)/AlgebraList.lo

.PHONY: clean_cs
clean_cs:
	$(MAKE) -C ClientServer clean
	$(MAKE) -C UserInterfaces clean_client	

.PHONY: clean_tests
clean_tests:
	$(MAKE) -C Tests clean

.PHONY: clean_all
clean_all: clean clean_tests clean_cs

.PHONY: distclean
distclean:
	$(MAKE) -C ClientServer distclean
	$(MAKE) -C StorageManager distclean
	$(MAKE) -C Tools distclean
	$(MAKE) -C Algebras distclean
	$(MAKE) -C QueryProcessor distclean
	$(MAKE) -C UserInterfaces distclean

.PHONY: help
help:
	@echo "*** Usage of the SECONDO makefile:"
	@echo "***"
	@echo "*** make [shared=yes] [target]"
	@echo "***"
	@echo "*** The optional definition shared=yes specifies"
	@echo "*** that shared libraries are built. Make sure that"
	@echo "*** the ./lib directory is in your LD_LIBRARY_PATH"
	@echo "*** variable (on windows the PATH variable). Consult"
	@echo "*** the readme file for deatiled information."
