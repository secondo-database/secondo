
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

# Folded in from the hand-written compile rules these replaced, so the %.o
# rule in makefile.env builds both objects and writes their .dep files.
CCFLAGS += -I../../ -I$(BDBINCLUDE)

.PHONY: all
all: Config.ini Queryexecutor$(EXEEXT) 


LDFLAGS:=-L${SECONDO_BUILD_DIR}/apis/api_cpp/cs/lib/ -lsecondo    -lfl -ly -lstdc++ 
ifeq ($(SECONDO_PLATFORM),win32)
LDFLAGS+=-lwsock32 -lws2_32
endif

ifneq ($(SECONDO_PLATFORM),mac_osx)
  LDFLAGS+=-L$(BDB_LIB_DIR) -l$(BERKELEY_DB_LIB)
else
  LDFLAGS+= -l$(BERKELEY_DB_LIB)
endif


LDFLAGS += -lpthread
LDFLAGS += -lssl -lcrypto -lz -luv -lcassandra
 
LDFLAGS += $(ALGDEPS)

Config.ini: $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini
	cp $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini ./Config.ini 
	sed -i -e "s#SecondoHome=.*#SecondoHome=$(CURDIR)/nltemp#g" Config.ini


Queryexecutor$(EXEEXT): queryexecutor.o heartbeat.o
	$(CPPC) -o Queryexecutor$(EXEEXT) queryexecutor.o heartbeat.o ../../CassandraAdapter.o ../../CassandraResult.o ../../CassandraTuplePrefetcher.o $(LDFLAGS)
	rm -rf nltemp
	mkdir nltemp


.PHONY:clean
clean:
	rm -f Config.ini Queryexecutor$(EXEEXT) *.o
	rm -f $(DEP_FILES)
	rm -rf nltemp
