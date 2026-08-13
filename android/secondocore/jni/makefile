
include ../../../makefile.env

#ifdef SECONDO_ANDROID
DEFAULTLIB=
#endif

ifndef BDB_INCLUDE_DIR
  BDB_INCLUDE_DIR=$(BERKELEY_DB_DIR)/include
endif

ifndef BDB_LIB_DIR
  BDB_LIB_DIR=$(BERKELEY_DB_DIR)/lib
endif

ifndef $(BERKELEY_DB_LIB)
  BERKELEY_DB_LIB=db_cxx # special patch for Simone
endif



.PHONY: all
all: Config.ini app$(EXEEXT)



Config.ini: $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini
	cp $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini ./Config.ini 
	sed -i -e "s#SecondoHome=.*#SecondoHome=$(CURDIR)/databases#g" Config.ini



ALGDEPFLAGS=$(addprefix -l, $(ALGEBRA_DEPS))

# Extra library directories for the cross-build. These used to be hardcoded to
# a developer's home directory; set ANDROID_LIB_DIRS to wherever the
# cross-compiled jpeg/gsl/bison/flex actually live.
ANDROID_LIB_DIRS ?=

LDFLAGS:=-L../lib/ $(addprefix -L,$(ANDROID_LIB_DIRS)) -lsecondo -lfl  -ly  -lstdc++ $(ALGDEPFLAGS)  $(DEFAULTLIB)


ifneq ($(SECONDO_PLATFORM),win32)
  LDFLAGS+=-ldl
else
  LDFLAGS+=-lwsock32 -lws2_32
endif
 
ifneq ($(SECONDO_PLATFORM),mac_osx)
  LDFLAGS+= -L$(BDB_LIB_DIR) -l$(BERKELEY_DB_LIB)
else
  LDFLAGS+=-l$(BERKELEY_DB_LIB)
endif




app$(EXEEXT): Example.o
	$(CPPC) -o app$(EXEEXT) Example.o $(LDFLAGS)  
	mkdir -p databases


# Folded in from the compile rule this replaced, so the %.o rule in
# makefile.env builds the object and writes its .dep.
CCFLAGS += -I../include -I$(BDB_INCLUDE_DIR)

.PHONY:clean
clean:
	rm -f Config.ini app Example.o
