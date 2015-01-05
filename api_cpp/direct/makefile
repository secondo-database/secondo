

include ../../makefile.env

BD=$(SECONDO_BUILD_DIR)
INCL=$(BD)/include
LAT=$(BD)/Tools/ListsAndTables

Interface1=$(INCL)/SecondoInterface.h
Interface2=$(INCL)/Application.h

HEADERS1=$$($(CPPC) -MM -I$(INCL) -I$(LAT) $(Interface1) | sed "s/.*://"  | sed -e 's/\\//g' | tr -d "\n"    )
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
SECLIBS+=$(LIBD)/libappCommon.a
SECLIBS+=$(TYPEMAPLIB)
LIB1:=$(addprefix $(BD)/lib/lib, $(ALGBASE))
LIB2:=$(addsuffix .$(LIBEXT), $(LIB1))
SECLIBS+=$(LIB2)


# define object files
SECOBJ:=$(LIBD)/SecondoInterface.o
SECOBJ+=$(BD)/lib/AlgebraList.o

.PHONY: all
all: api



api: buildincludes buildlib
	make -C example all	

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
	ar -rs  lib/libsecondo.$(LIBEXT) $(SECOBJ) 
	@arlib $$PWD/lib/libsecondo.$(LIBEXT) $(SECOBJ)  $(SECLIBS)


.PHONY:clean
clean:
	rm -rf include
	rm -rf lib
	make -C example clean
	rm -rf temp



