include ../../../makefile.env

CC := g++
LD := g++

CCFLAGS += -DUSE_SERIALIZATION

#CCFLAGS += -I/e/Secondo/boost_1_57_0
CCFLAGS += -I../../Rectangle
CCFLAGS += -I../../Spatial
CCFLAGS += -I../../Relation-C++
CCFLAGS += -I../../KeyValueStore

CCFLAGS += $(DEFAULTCCFLAGS)

#not sure
#@chmod u+x ../bin/KeyValueStore

KVS_FILES = $(wildcard *.cpp)
KVS_OBJECTS = $(KVS_FILES:.cpp=.o)

ALG_LIST :=  SecondoInterfaceTTY.o  SecondoInterfaceCS.o AlgebraList.o SecondoInterfaceGeneral.o
APP_LIB := ../../../lib/libappCommon.$(LIBEXT)

# KeyValueStore, the stand alone application.
$(BINDIR)/KeyValueStore$(EXEEXT): $(APP_LIB) $(ALG_LIST) $(KVS_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o, $^) -lappCommon $(LD_LINK_LIBS_ALG) $(RL_LD_FLAG) $(PLLDFLAGS) $(COMMON_LD_FLAGS)

%.o: %.cpp
	$(CC) -c $(CCFLAGS) $< -o $@

clean:
	rm -f $(KVS_OBJECTS) $(BINDIR)/KeyValueStore$(EXEEXT)
