
include ../../makefile.env

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


LDFLAGS+=-L../../apis/api_cpp/cs/lib -lsecondo -ly
ifeq ($(SECONDO_PLATFORM),win32)
LDFLAGS+=-lwsock32 -lws2_32
endif

LDFLAGS+=-L$(BDB_LIB_DIR) -l$(BERKELEY_DB_LIB)


Config.ini: $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini
	cp $(SECONDO_BUILD_DIR)/bin/SecondoConfig.ini ./Config.ini 


app$(EXEEXT): ClientLoadTest.o
	$(CPPC) -o app$(EXEEXT) ClientLoadTest.o  $(LDFLAGS) $(DEFAULTLIB) $(COMMON_LD_FLAGS)


ClientLoadTest.o: ClientLoadTest.cpp
	$(CPPC) -c -o ClientLoadTest.o $(CCFLAGS)  ClientLoadTest.cpp -I../include -I$(BDBINCLUDE)

########################################################################
#
# Concurrency contract checker: "make app-check", then ./app-check
#
# The other half of NestedList's threading contract -- "operations on the
# same list need the caller's own lock" -- is the caller's to keep, so it
# cannot be checked by construction. -DNL_CHECK_CONCURRENCY compiles in a
# checker that turns breaking it into an immediate abort naming both threads
# and the node, and --mode contract breaks it on purpose.
#
# It is not a sanitizer build: ThreadSanitizer catches accesses that race but
# do not overlap in time, this catches overlapping ones, and the two are
# complementary. TSan comes from a whole-tree "make SECONDO_TSAN=1" instead
# (see makefile.env), which is why nothing here has to be instrumented.
#
# The flag reaches exactly two translation units. NodeAccessGuard is a free
# class, not a NestedList member, so the class layout does not change and
# the rest of libsecondo.a can be used as built; and the NL_WRITING_NODE /
# NL_READING_NODE macros appear only in NestedList.cpp. The test itself needs
# it for --mode contract.
#
########################################################################

BD=$(SECONDO_BUILD_DIR)

CHECK_FLAGS = -DNL_CHECK_CONCURRENCY
CHECK_INCLUDES = -I$(BD)/Tools/NestedLists -I../include -I$(BDBINCLUDE)

check/ClientLoadTest.o: ClientLoadTest.cpp
	@mkdir -p check
	$(CPPC) -c -o $@ $(CCFLAGS) $(CHECK_FLAGS) $(CHECK_INCLUDES) $< \
	    -MMD -MP -MF check/ClientLoadTest.dep

check/NestedList.o: $(BD)/Tools/NestedLists/NestedList.cpp
	@mkdir -p check
	$(CPPC) -c -o $@ $(CCFLAGS) $(CHECK_FLAGS) $(CHECK_INCLUDES) $< \
	    -MMD -MP -MF check/NestedList.dep

-include $(wildcard check/*.dep)

# Our NestedList.o is named ahead of -lsecondo (which LDFLAGS supplies), so
# every NestedList symbol is already resolved by the time the linker reaches
# the archive and its own uninstrumented copy is never pulled in.
app-check$(EXEEXT): check/ClientLoadTest.o check/NestedList.o
	$(CPPC) -o $@ check/ClientLoadTest.o check/NestedList.o \
	    $(LDFLAGS) $(DEFAULTLIB) $(COMMON_LD_FLAGS)

.PHONY: check
check: Config.ini app-check$(EXEEXT)

.PHONY:clean
clean:
	rm -f Config.ini app$(EXEEXT) ClientLoadTest.o
	rm -rf check app-check$(EXEEXT)
