

include ../../../makefile.env

BD=$(SECONDO_BUILD_DIR)
INCL=$(BD)/include
LAT=$(BD)/Tools/ListsAndTables

Interface1=$(INCL)/SecondoInterface.h $(INCL)/SecondoInterfaceTTY.h $(INCL)/SecondoInterfaceCS.h 
Interface2=$(INCL)/Application.h



HEADERS1=$$($(CPPC) -MM -I$(INCL) -I$(LAT) $(Interface1) $(DEFAULTCCFLAGS) | sed "s/.*://"  | sed -e 's/\\//g' | tr "[:blank:]" "\n" | sort | uniq | tr "\n" " " )
DEPS1=$(HEADERS1)

ifeq ($(platform),win32)
USEWIN:=-D SECONDO_WIN32
endif


HEADERS2 := $$($(CPPC) -MM -D CTABLE_PERSISTENT -D NL_PERSISTENT $(USEWIN)  -I$(INCL) -I$(LAT) $(Interface1) $(DEFAULTCCFLAGS) | sed "s/.*://"  | sed -e 's/\\//g' |  tr "[:blank:]" "\n" | sort | uniq | tr "\n" " " )
DEPS2=$(HEADERS2)


LIBD=$(BD)/lib


# define lib files
SECLIBS:=$(LIBD)/libsdbnl.a
SECLIBS+=$(LIBD)/libsdbsocket.a
SECLIBS+=$(LIBD)/libsdbsys.a
SECLIBS+=$(LIBD)/libsdbutils.a
SECLIBS+=$(LIBD)/libsmibdb.a
SECLIBS+=$(LIBD)/libappCommon.a
SECLIBS+=$(TYPEMAPLIB)
LIB1:=$(addprefix $(BD)/lib/lib, $(ALGBASE))
LIB2:=$(addsuffix .$(LIBEXT), $(LIB1))
SECLIBS+=$(LIB2)


# define object files
SECOBJ:=$(LIBD)/SecondoInterfaceGeneral.o $(LIBD)/SecondoInterfaceTTY.o $(LIBD)/SecondoInterfaceCS.o 
SECOBJ+=$(BD)/lib/AlgebraList.o





.PHONY: all
all: api



api: buildincludes buildlib
	$(MAKE) -C example all	

#max osx does ot support the -u option for cp command
ifneq ($(SECONDO_PLATFORM),mac_osx)
  CPOPTION=-u
endif

buildincludes: 
	mkdir -p include 
	rm -rf include/*.h
	cp $(CPOPTION) $(DEPS1) include
	cp $(CPOPTION) $(DEPS2) include

buildlib: lib lib/libsecondo.$(LIBEXT)

lib:
	mkdir lib



lib/libsecondo.$(LIBEXT): $(SECLIBS) lib
	$(AR) -rs  lib/libsecondo.$(LIBEXT) $(SECOBJ) 
	@./arlib $(AR) $(CURDIR)/lib/libsecondo.$(LIBEXT) $(SECOBJ)  $(SECLIBS)


.PHONY:clean
clean:
	rm -rf include/*.h
	rm -rf lib
	$(MAKE) -C example clean
	rm -rf temp


