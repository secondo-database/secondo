

include ../../makefile.env

BD=$(SECONDO_BUILD_DIR)
INCL=$(BD)/include
LAT=$(BD)/Tools/ListsAndTables

Interface1=$(INCL)/SecondoInterface.h
Interface2=$(INCL)/Application.h

HEADERS1=$$($(CPPC) -MM -I$(INCL) -I$(LAT) $(Interface1) $(DEFAULTCCFLAGS)  | sed "s/.*://"  | sed -e 's/\\//g' | sort | uniq | tr -d "\n"    )
DEPS1=$(HEADERS1)


HEADERS2=$$($(CPPC) -MM -D CTABLE_PERSISTENT -D NL_PERSISTENT -D SECONDO_WIN32 -I$(INCL) -I$(LAT) $(Interface1) $(DEFAULTCCFLAGS) | sed "s/.*://"  | sed -e 's/\\//g' | sort | uniq | tr -d "\n"    )
DEPS2=$(HEADERS2)


LIBD=$(BD)/lib

CSLIBS+=$(LIBD)/SecondoInterfaceCS.o
CSLIBS+=$(LIBD)/SecondoInterfaceGeneral.o
CSLIBS+=$(BD)/Tools/NestedLists/NestedList.o
CSLIBS+=$(BD)/Tools/Utilities/WinUnix.o
CSLIBS+=$(BD)/Tools/Utilities/Environment.o
CSLIBS+=$(BD)/UserInterfaces/cmsg.o
CSLIBS+=$(BD)/Tools/NestedLists/NList.o
CSLIBS+=$(BD)/Tools/Utilities/FileSystem.o
CSLIBS+=$(BD)/Tools/Utilities/UtilFunctions.o
CSLIBS+=$(BD)/Tools/Utilities/Application.o
CSLIBS+=$(BD)/StorageManager/*.o
CSLIBS+=$(BD)/ClientServer/SocketIO.o
CSLIBS+=$(BD)/Tools/Utilities/Profiles.o
CSLIBS+=$(BD)/Tools/Utilities/Messenger.o
CSLIBS+=$(BD)/Tools/NestedLists/NLParser.o
CSLIBS+=$(BD)/Tools/NestedLists/NLScanner.o
CSLIBS+=$(BD)/Tools/NestedLists/NLLex.o
CSLIBS+=$(BD)/Tools/NestedLists/NLParser.tab.o
CSLIBS+=$(BD)/Tools/Utilities/StringUtils.o
CSLIBS+=$(BD)/Tools/Utilities/Base64.o
CSLIBS+=$(BD)/Tools/Flob/ExternalFileCache.o
CSLIBS+=$(BD)/Tools/Flob/FlobManager.o
CSLIBS+=$(BD)/Tools/Flob/PersistentFlobCache.o




ifneq ($(SECONDO_PLATFORM),mac_osx)
  CPOPTION=-u
endif



.PHONY: all
all: api



api: buildincludes buildlib
	make -C example



buildincludes: 
	@rm -rf include 
	@mkdir include >/dev/null 2>&1; echo""
	cp $(CPOPTION) $(DEPS1) include
	cp $(CPOPTION) $(DEPS2) include

buildlib: lib lib/libsecondo.$(LIBEXT)

lib:
	mkdir lib

lib/libsecondo.$(LIBEXT): $(CSLIBS)
	ar -rs  lib/libsecondo.$(LIBEXT) $(CSLIBS) 


.PHONY:clean
clean:
	rm -rf include
	rm -rf lib
	make -C example clean



