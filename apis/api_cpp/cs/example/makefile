
include ../../../../makefile.env

ifndef BDB_INCLUDE_DIR
BDBINCLUDE=$(BERKELEY_DB_DIR)/include
else 
  BDBINCLUDE=$(BDB_INCLUDE_DIR)
endif  

ifndef BDB_LIB_DIR
BDB_LIB_DIR=$(BERKELEY_DB_DIR)/lib
endif

ifndef $(BERKELEY_DB_LIB)
  BERKELEY_DB_LIB=db_cxx # special patch for Simone
endif

.PHONY: all
all: Config.ini app$(EXEEXT)


LDFLAGS+=-L../lib/ -lsecondo -lstdc++
ifeq ($(SECONDO_PLATFORM),win32)
LDFLAGS+=-lwsock32 -lws2_32
endif

#ifneq ($(SECONDO_PLATFORM),mac_osx)
LDFLAGS+=-L$(BDB_LIB_DIR) -l$(BERKELEY_DB_LIB)
#else
#  LDFLAGS+= -l$(BERKELEY_DB_LIB)
#endif


LDFLAGS += -lpthread

Config.ini: $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini
	cp $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini ./Config.ini 


app$(EXEEXT): Example.o
	$(CPPC) -o app$(EXEEXT) Example.o  $(LDFLAGS) $(DEFAULTLIB) $(COMMON_LD_FLAGS)


Example.o: Example.cpp
	$(CPPC) -c -o Example.o $(CCFLAGS)  Example.cpp -I../include -I$(BDBINCLUDE) 

.PHONY:clean
clean:
	rm -f Config.ini app$(EXEEXT) Example.o
