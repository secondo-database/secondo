include ../makefile.env

DEFAULTLIB=

BD=$(SECONDO_BUILD_DIR)
INCL=$(BD)/include
LAT=$(BD)/Tools/ListsAndTables

Interface1=$(INCL)/SecondoInterface.h
Interface2=$(INCL)/Application.h

HEADERS1=$$($(CPPC) -MM -I$(INCL) -I$(LAT) $(Interface1) | sed "s/.*://"  | sed -e 's/\\//g' | tr -d "\n" )
DEPS1=$(HEADERS1)


HEADERS2=$$($(CPPC) -MM -D CTABLE_PERSISTENT -D NL_PERSISTENT -D SECONDO_WIN32 -I$(INCL) -I$(LAT) $(Interface1) | sed "s/.*://"  | sed -e 's/\\//g' | tr -d "\n"    )
DEPS2=$(HEADERS2)


LIBD=$(BD)/lib


# define lib files
SECLIBS:=$(LIBD)/libsdbnl.a
SECLIBS+=$(LIBD)/libsdbsocket.a
SECLIBS+=$(LIBD)/libsdbsys.a
SECLIBS+=$(LIBD)/libsdbutils.a
SECLIBS+=$(LIBD)/libsmibdb.a
LIB1:=$(addprefix $(BD)/lib/lib, $(ALGBASE))
LIB2:=$(addsuffix .$(LIBEXT), $(LIB1))
SECLIBS+=$(LIB2)


# define object files
SECOBJ:=$(LIBD)/SecondoInterface.o
SECOBJ+=$(LIBD)/SecondoInterfaceGeneral.o
SECOBJ+=$(LIBD)/SecondoInterface.o
SECOBJ+=$(BD)/Tools/NestedLists/NestedList.o
SECOBJ+=$(BD)/Tools/Utilities/*.o
SECOBJ+=$(BD)/UserInterfaces/cmsg.o
SECOBJ+=$(BD)/Tools/NestedLists/NList.o
SECOBJ+=$(BD)/StorageManager/*.o
SECOBJ+=$(BD)/ClientServer/SocketIO.o
SECOBJ+=$(BD)/Tools/NestedLists/NLParser.o
SECOBJ+=$(BD)/Tools/NestedLists/NLScanner.o
SECOBJ+=$(BD)/Tools/NestedLists/NLLex.o
SECOBJ+=$(BD)/Tools/NestedLists/NLParser.tab.o
SECOBJ+=$(BD)/Tools/Flob/FlobManager.o
SECOBJ+=$(BD)/Tools/Flob/PersistentFlobCache.o
SECOBJ+=$(BD)/QueryProcessor/*.o
SECOBJ+=$(BD)/Algebras/Management/AlgebraManager.o
SECOBJ+=$(BD)/Algebras/Management/Algebra.o
SECOBJ+=$(BD)//Tools/Parser/*.o
SECOBJ+=$(BD)/Tools/Parser/SpecParser/NestedText.o
SECOBJ+=$(BD)/./Tools/NestedLists/ListUtils.o
SECOBJ+=$(BD)/lib/AlgebraList.o

.PHONY: all
all: api



api: buildincludes buildlib

#max osx does ot support the -u option for cp command
ifneq ($(SECONDO_PLATFORM),mac_osx)
	CPOPTION=-u
endif

buildincludes: 
	@rm -rf include 
	@mkdir include >/dev/null 2>&1; echo""
	cp $(CPOPTION) $(DEPS1) include
	cp $(CPOPTION) $(DEPS2) include

buildlib: lib lib/libsecondo.$(LIBEXT)

lib:
	mkdir lib

lib/libsecondo.$(LIBEXT): $(SECLIBS)
	$(AR) -rs  lib/libsecondo.$(LIBEXT) $(SECOBJ) 
	./arlib $(CURDIR)/lib/libsecondo.$(LIBEXT) $(SECLIBS)
#       @arlib $(CURDIR)/lib/libsecondo.$(LIBEXT) $(SECLIBS)


.PHONY:clean
clean:
	rm -rf include
	rm -rf lib
	rm -rf temp

