
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
# ThreadSanitizer build: "make tsan", then ./app-tsan
#
# Checks that the client library tolerates several connections in one
# process. Everything the test links has to be instrumented for the report
# to mean anything, so the client sources are recompiled here rather than
# taken from libsecondo.a -- into ./tsan/, so that the objects the normal
# build produced are left alone. Mixing instrumented and uninstrumented
# objects is what produces phantom reports.
#
########################################################################

BD=$(SECONDO_BUILD_DIR)

# The sources behind libsecondo.a. Mirrors CSLIBS in apis/api_cpp/cs/makefile;
# "make tsan-check-sources" fails if the two have drifted apart.
CS_SOURCES = \
  $(BD)/ClientServer/SecondoInterfaceCS.cpp \
  $(BD)/ClientServer/SocketIO.cpp \
  $(BD)/QueryProcessor/SecondoInterfaceGeneral.cpp \
  $(BD)/Tools/BigArray/BigArray.cpp \
  $(BD)/Tools/NestedLists/NestedList.cpp \
  $(BD)/Tools/NestedLists/NList.cpp \
  $(BD)/Tools/NestedLists/NLParser.cpp \
  $(BD)/Tools/NestedLists/NLScanner.cpp \
  $(BD)/Tools/NestedLists/NLLex.cpp \
  $(BD)/Tools/NestedLists/NLParser.tab.cpp \
  $(BD)/Tools/Utilities/WinUnix.cpp \
  $(BD)/Tools/Utilities/Environment.cpp \
  $(BD)/Tools/Utilities/FileSystem.cpp \
  $(BD)/Tools/Utilities/UtilFunctions.cpp \
  $(BD)/Tools/Utilities/Application.cpp \
  $(BD)/Tools/Utilities/Profiles.cpp \
  $(BD)/Tools/Utilities/Messenger.cpp \
  $(BD)/Tools/Utilities/StringUtils.cpp \
  $(BD)/Tools/Utilities/Base64.cpp \
  $(BD)/Tools/Utilities/AlmostEqual.cpp \
  $(BD)/Tools/Utilities/satof.cpp \
  $(BD)/Tools/Flob/ExternalFileCache.cpp \
  $(BD)/Tools/Flob/FlobManager.cpp \
  $(BD)/Tools/Flob/PersistentFlobCache.cpp \
  $(BD)/UserInterfaces/cmsg.cpp \
  $(BD)/UserInterfaces/SQLLanguage.cpp

TSAN_LIB_OBJECTS = $(addprefix tsan/,$(notdir $(CS_SOURCES:.cpp=.o)))

# -O1 keeps the stack frames that make a TSan report readable; the sanitizer
# dominates the run time in any case.
TSAN_FLAGS = -fsanitize=thread -fno-omit-frame-pointer -g -O1
TSAN_INCLUDES = -I$(BD)/Tools/NestedLists -I../include -I$(BDBINCLUDE)

vpath %.cpp $(sort $(dir $(CS_SOURCES)))

tsan/%.o: %.cpp
	@mkdir -p tsan
	$(CPPC) -c -o $@ $(CCFLAGS) $(TSAN_FLAGS) $(TSAN_INCLUDES) $< \
	    -MMD -MP -MF tsan/$*.dep

tsan/ClientLoadTest.o: ClientLoadTest.cpp
	@mkdir -p tsan
	$(CPPC) -c -o $@ $(CCFLAGS) $(TSAN_FLAGS) $(TSAN_INCLUDES) $< \
	    -MMD -MP -MF tsan/ClientLoadTest.dep

-include $(wildcard tsan/*.dep)

.PHONY: tsan
tsan: Config.ini app-tsan$(EXEEXT)

# Archived rather than linked object by object, exactly as libsecondo.a is:
# several members (the Flob caches) reference the storage interface, which a
# client never calls. Out of an archive the linker simply does not pull them
# in; listing every object instead would demand symbols nothing uses.
tsan/libsecondo-tsan.a: $(TSAN_LIB_OBJECTS)
	$(AR) -rs $@ $(TSAN_LIB_OBJECTS)

app-tsan$(EXEEXT): tsan/ClientLoadTest.o tsan/libsecondo-tsan.a
	$(CPPC) -o $@ tsan/ClientLoadTest.o -Ltsan -lsecondo-tsan $(TSAN_FLAGS) \
	    -L$(BDB_LIB_DIR) -l$(BERKELEY_DB_LIB) -lpthread -lstdc++ \
	    $(COMMON_LD_FLAGS)

# Guards the list above against the archive growing a member we do not
# instrument -- a silently uninstrumented object makes TSan quietly blind.
.PHONY: tsan-check-sources
tsan-check-sources:
	@ar t $(BD)/apis/api_cpp/cs/lib/libsecondo.a | sort > /tmp/cslt_ar.$$$$; \
	 for s in $(CS_SOURCES); do basename $$s .cpp; done \
	   | sed 's/$$/.o/' | sort > /tmp/cslt_ours.$$$$; \
	 if diff -u /tmp/cslt_ar.$$$$ /tmp/cslt_ours.$$$$; then \
	   echo "tsan source list matches libsecondo.a"; rc=0; \
	 else \
	   echo "tsan source list has drifted from apis/api_cpp/cs/makefile"; rc=1; \
	 fi; \
	 rm -f /tmp/cslt_ar.$$$$ /tmp/cslt_ours.$$$$; exit $$rc

.PHONY:clean
clean:
	rm -f Config.ini app$(EXEEXT) ClientLoadTest.o
	rm -rf tsan app-tsan$(EXEEXT)
