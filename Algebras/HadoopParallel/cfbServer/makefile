
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
all: collectFlobServer$(EXEEXT)

LDFLAGS:= -lpthread

collectFlobServer$(EXEEXT): collectFlobServer.o
	$(CPPC) -o collectFlobServer$(EXEEXT) collectFlobServer.o $(SECONDO_BUILD_DIR)/Tools/Utilities/Profiles.o $(SECONDO_BUILD_DIR)/Tools/Utilities/UtilFunctions.o $(SECONDO_BUILD_DIR)/Tools/Utilities/WinUnix.o $(LDFLAGS) $(COMMON_LD_FLAGS)
	cp collectFlobServer$(EXEEXT) $(SECONDO_BUILD_DIR)/bin

.PHONY:clean
clean:
	rm -f collectFlobServer$(EXEEXT) collectFlobServer.o $(DEP_FILES)

