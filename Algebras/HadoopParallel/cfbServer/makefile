
include ../../../makefile.env

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
all: collectFlobServer$(EXEEXT)

ALGDEPFLAGS=$(addprefix -l, $(ALGEBRA_DEPS))

LDFLAGS:=-L$(SECONDO_BUILD_DIR)/api_cpp/direct/lib/ -lsecondo -lfl -ly -lstdc++ $(ALGDEPFLAGS)  $(DEFAULTLIB)

ifeq ($(readline),true)
  LDFLAGS+=-lreadline 
endif

ifeq ($(SECONDO_PLATFORM),win32)
  LDFLAGS+=-lwsock32 -lws2_32
else
  LDFLAGS+=-ldl
endif

ifneq ($(SECONDO_PLATFORM),mac_osx)
  LDFLAGS+=-L$(BDB_LIB_DIR) -l$(BERKELEY_DB_LIB)
else
  LDFLAGS+= -l$(BERKELEY_DB_LIB)
endif

LDFLAGS += -lpthread

collectFlobServer$(EXEEXT): collectFlobServer.o
	$(CPPC) -o collectFlobServer$(EXEEXT) collectFlobServer.o  $(LDFLAGS)
	cp collectFlobServer$(EXEEXT) $(SECONDO_BUILD_DIR)/bin

collectFlobServer.o: collectFlobServer.cpp
	$(CPPC) -c -o collectFlobServer.o $(CCFLAGS) collectFlobServer.cpp -I../include -I$(BDBINCLUDE) 

.PHONY:clean
clean:
	rm -f collectFlobServer$(EXEEXT) collectFlobServer.o

