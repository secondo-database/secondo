
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

# Folded in from what used to be a hand-written compile rule, so the
# %.o rule in makefile.env can build this object -- and write its .dep.
CCFLAGS += -I../include -I$(BDBINCLUDE)

.PHONY: all
all: app$(EXEEXT)


LDFLAGS:=-L$(SECONDO_BUILD_DIR)/api_cpp/cs/lib/ -lsecondo    -lfl -ly -lstdc++ 
ifeq ($(SECONDO_PLATFORM),win32)
LDFLAGS+=-lwsock32 -lws2_32
endif

ifneq ($(SECONDO_PLATFORM),mac_osx)
  LDFLAGS+=-L$(BDB_LIB_DIR) -l$(BERKELEY_DB_LIB)
else
  LDFLAGS+= -l$(BERKELEY_DB_LIB)
endif


LDFLAGS += -lpthread

app$(EXEEXT): askFlob.o
	$(CPPC) -o askFlob$(EXEEXT) askFlob.o  $(LDFLAGS)
	cp askFlob$(EXEEXT) $(SECONDO_BUILD_DIR)/bin

.PHONY:clean
clean:
	rm -f askFlob$(EXEEXT) askFlob.o $(DEP_FILES)

