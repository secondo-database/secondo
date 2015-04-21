
include ../../../makefile.env

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
	sed -i -e "s#SecondoHome=.*#SecondoHome=$$PWD/databases#g" Config.ini



#ALGDEPFLAGS=$(addprefix -l, $(ALGEBRA_DEPS))


LDFLAGS+=-L../lib/ -lsecondo  -lfl -ly  -lstdc++ $(ALGDEPS)  $(DEFAULTLIB)

ifeq ($(readline),true)
  LDFLAGS+=-lreadline 
endif



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

LDFLAGS += -lpthread



app$(EXEEXT): Example.o
	$(CPPC) -o app$(EXEEXT) Example.o $(LDFLAGS) $(COMMON_LD_FLAGS) 
	mkdir -p databases


Example.o: Example.cpp
	$(CPPC) -c -o Example.o $(CCFLAGS) Example.cpp -I../include -I$(BDB_INCLUDE_DIR) 

.PHONY:clean
clean:
	rm -f Config.ini app Example.o
