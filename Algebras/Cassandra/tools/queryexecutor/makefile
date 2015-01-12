
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
all: Config.ini Queryexecutor$(EXEEXT) Algebra


LDFLAGS:=-L../../../../api_cpp/cs/lib/ -lsecondo    -lfl -ly -lstdc++ 
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

Algebra:
	make -C ../../

Config.ini: $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini
	cp $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini ./Config.ini 
	sed -i -e "s#SecondoHome=.*#SecondoHome=$$PWD/nltemp#g" Config.ini


Queryexecutor$(EXEEXT): Queryexecutor.o heartbeat.o
	$(CPPC) -o Queryexecutor$(EXEEXT) Queryexecutor.o heartbeat.o ../../CassandraAdapter.o ../../CassandraResult.o $(LDFLAGS)
	rm -rf nltemp
	mkdir nltemp

heartbeat.o: heartbeat.cpp 
	$(CPPC) -c -o heartbeat.o $(CCFLAGS) heartbeat.cpp -I../../ -I$(BDBINCLUDE) 

Queryexecutor.o: Queryexecutor.cpp Algebra 
	$(CPPC) -c -o Queryexecutor.o $(CCFLAGS) Queryexecutor.cpp -I../../ -I$(BDBINCLUDE) 


.PHONY:clean
clean:
	rm -f Config.ini app$(EXEEXT) *.o
	rm -rf nltemp
