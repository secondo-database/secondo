########################################################################
#
# SECONDO Makefile
#
########################################################################

include makefile.env

#libsdbtool
LIB_SDBTOOL_BASENAMES=\
	Tools/NestedLists/NestedList \
	Tools/NestedLists/NLLex \
	Tools/NestedLists/NLScanner \
	Tools/NestedLists/NLParser \
	Tools/NestedLists/NLParser.tab \
	Tools/Parser/SecLex \
	Tools/Parser/SecParser \
	Tools/Parser/SecParser.tab \
	Tools/Parser/NestedText 

#libsdbutils
LIB_SDBUTILS_BASENAMES=\
	Tools/Utilities/Processes \
	Tools/Utilities/Application \
	Tools/Utilities/Messenger \
	Tools/Utilities/DynamicLibrary \
	Tools/Utilities/FileSystem \
	Tools/Utilities/Profiles \
	Tools/Utilities/UtilFunctions \
	Tools/Utilities/WinUnix \

#libsdbsocket
LIB_SDBSOCKET_BASENAMES=\
	ClientServer/SocketIO \
	ClientServer/SocketAddress \
	ClientServer/SocketRuleSet

#libsdbsys
LIB_SDBSYS_BASENAMES=\
	Algebras/Management/Algebra \
	Algebras/Management/AlgebraManager \
	QueryProcessor/QueryProcessor \
	QueryProcessor/SecondoSystem \
	QueryProcessor/SecondoCatalog


LIB_SDBTOOL_OBJECTS   = $(addsuffix .$(OBJEXT), $(LIB_SDBTOOL_BASENAMES))
LIB_SDBUTILS_OBJECTS  = $(addsuffix .$(OBJEXT), $(LIB_SDBUTILS_BASENAMES))
LIB_SDBSOCKET_OBJECTS = $(addsuffix .$(OBJEXT), $(LIB_SDBSOCKET_BASENAMES))
LIB_SDBSYS_OBJECTS    = $(addsuffix .$(OBJEXT), $(LIB_SDBSYS_BASENAMES))

ifeq ($(smitype),ora)
SMILIB=$(ORASMILIB)
else
SMILIB=$(BDBSMILIB)
endif

.PHONY: all
all: makedirs buildlibs buildalg buildapps java showjni

.PHONY: showjni
showjni:
	@echo $(JNITEXT)
	
.PHONY: javagui
javagui:
	$(MAKE) -C Javagui all	

.PHONY: clientserver
clientserver: cs

.PHONY: cs 
cs: makedirs buildlibs buildalg
	$(MAKE) -C ClientServer
	$(MAKE) -C UserInterfaces TTYCS 
	$(MAKE) -C ClientServer buildapp


.PHONY: makedirs
makedirs:
	$(MAKE) -C ClientServer
	$(MAKE) -C Tools
	$(MAKE) -C StorageManager
	$(MAKE) -C Algebras/Management
	$(MAKE) -C QueryProcessor
	$(MAKE) -C UserInterfaces

.PHONY: java
java:
	$(MAKE) -C Javagui all

.PHONY: TTY showjni 
TTY: makedirs buildlibs buildalg
	$(MAKE) -C UserInterfaces TTY 

.PHONY: TestRunner showjni
TestRunner: makedirs buildlibs buildalg
	$(MAKE) -C UserInterfaces TestRunner


.PHONY: optimizer
optimizer: makedirs buildlibs buildalg
	$(MAKE) -C UserInterfaces optimizer
	
.PHONY: buildlibs
buildlibs: $(LIBDIR)/libsdbtool.$(LIBEXT) $(LIBDIR)/libsdbutils.$(LIBEXT) buildsmilibs $(LIBDIR)/libsdbsys.$(LIBEXT) $(LIBDIR)/libsdbsocket.$(LIBEXT)

	$(MAKE) -C Algebras buildlibs

.PHONY: buildsmilibs
buildsmilibs:
	$(MAKE) -C StorageManager buildlibs

.PHONY: buildalg
buildalg:
	$(MAKE) -C Algebras


# --- Secondo Database Tools library ---

# Windows needs special treatment for dynamic libraries!
# The variable LIBNAME will be used in the LDOPTTOOL variable 
# which is only defined makefile.win32 included by makefile.env.

$(LIBDIR)/libsdbtool.$(LIBEXT): LIBNAME=libsdbtool
$(LIBDIR)/libsdbtool.$(LIBEXT): $(LIB_SDBTOOL_OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/libsdbtool.$(LIBEXT) $(LDOPT) $(LIB_SDBTOOL_OBJECTS) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/libsdbtool.$(LIBEXT) $^ 
endif

# --- Secondo Database Utilities Library ---

LIBNAME=libsdbutils
$(LIBDIR)/libsdbutils.$(LIBEXT): $(LIB_SDBUTILS_OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/libsdbutils.$(LIBEXT) $(LDOPTTOOL) $(LIB_SDBUTILS_OBJECTS) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/libsdbutils.$(LIBEXT) $^
endif

# --- Secondo Database System Library ---

$(LIBDIR)/libsdbsys.$(LIBEXT): LIBNAME=libsdbsys
$(LIBDIR)/libsdbsys.$(LIBEXT): $(LIB_SDBSYS_OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/libsdbsys.$(LIBEXT) $(LDOPT) $(LIB_SDBSYS_OBJECTS) -L$(LIBDIR) $(BDBSMILIB) $(SMILIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/libsdbsys.$(LIBEXT) $^ 
endif


# --- Secondo Database Socket Library ---

$(LIBDIR)/libsdbsocket.$(LIBEXT): LIBNAME=libsdbsocket
$(LIBDIR)/libsdbsocket.$(LIBEXT): $(LIB_SDBSOCKET_OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(LDOPT) $^ -L$(LIBDIR) $(BDBSMILIB) $(SMILIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/$(LIBNAME).$(LIBEXT) $^
endif


# --- Applications

.PHONY: buildapps
buildapps: 
	$(MAKE) -C UserInterfaces buildapp
	$(MAKE) -C ClientServer buildapp

.PHONY: tests
tests: makedirs buildlibs
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
