########################################################################
#
# SECONDO makefile for the Storage Management Interface
#
########################################################################

include ../makefile.env

BDBOBJECTSBASENAMES :=\
	smiEnvironment \
	smiKey \
	bdbKeyedFile \
	bdbRecordFile \
	bdbFile \
	bdbRecord \
	bdbEnvironment

ORAOBJECTSBASENAMES :=\
	smiEnvironment \
	smiKey \
	oraKeyedFile \
	oraRecordFile \
	oraFile \
	oraRecord \
	oraEnvironment

BDBOBJECTS := $(addsuffix .$(OBJEXT), $(BDBOBJECTSBASENAMES)) 
ORAOBJECTS := $(addsuffix .$(OBJEXT), $(ORAOBJECTSBASENAMES))

SMIHDR := SecondoConfig.h SecondoSMI.h SmiCodes.h Profiles.h
ORASMIHDR := SmiORA.h $(SMIHDR)
BDBSMIHDR := SmiBDB.h $(SMIHDR)

# Windows needs special treatment for dynamic libraries!
# The variable LIBNAME will be used in the LDOPTTOOL variable 
# which is only defined makefile.win32 included by makefile.env.
ifeq ($(smitype),ora)
CCFLAGS := $(PICFLAG) $(OCICPPINCLUDE) $(DEFAULTCCFLAGS)
ALLOBJ := $(ORAOBJECTS)
LIBNAME := libsmiora 
else
CCFLAGS := $(PICFLAG) $(BDBINCLUDE) $(DEFAULTCCFLAGS)
ALLOBJ := $(BDBOBJECTS)
LIBNAME := libsmibdb
endif

.PHONY: all
all: $(ALLOBJ)

.PHONY: buildlibs
buildlibs: $(LIBNAME).$(LIBEXT)

# --- Storage Management Interface library ---

$(LIBDIR)/libsmibdb.$(LIBEXT): $(BDBOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $@ $(LDOPT) $(BDBOBJECTS) $(BDBLIBDIR) $(BDBLIB) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $@ $(BDBOBJECTS)
endif

$(LIBDIR)/libsmiora.$(LIBEXT): $(ORAOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $@ $(LDOPT) $(ORAOBJECTS) $(OCICPPLIBDIR) $(OCICPPLIB) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $@ $(ORAOBJECTS)
endif

# --- Storage Management Interface modules ---

smiEnvironment.$(OBJEXT): smiEnvironment.cpp $(SMIHDR) Messenger.h FileSystem.h
	$(CC) -c -o $@ $< $(CCFLAGS)

smiKey.$(OBJEXT): smiKey.cpp $(SMIHDR)
	$(CC) -c -o $@ $<  $(CCFLAGS)

bdbEnvironment.$(OBJEXT): bdbEnvironment.cpp $(BDBSMIHDR)  FileSystem.h
	$(CC) -c -o $@ $<  $(CCFLAGS)

bdbFile.$(OBJEXT): bdbFile.cpp $(BDBSMIHDR)
	$(CC) -c -o $@ $<  $(CCFLAGS)

bdbRecordFile.$(OBJEXT): bdbRecordFile.cpp $(BDBSMIHDR)
	$(CC) -c -o $@ $<  $(CCFLAGS)

bdbKeyedFile.$(OBJEXT): bdbKeyedFile.cpp $(BDBSMIHDR)
	$(CC) -c -o $@ $<  $(CCFLAGS)

bdbRecord.$(OBJEXT): bdbRecord.cpp $(BDBSMIHDR)
	$(CC) -c -o $@ $<  $(CCFLAGS)

oraEnvironment.$(OBJEXT): oraEnvironment.cpp $(ORASMIHDR)
	$(CC) -c -o $@ $<  $(CCFLAGS)

oraFile.$(OBJEXT): oraFile.cpp $(ORASMIHDR)
	$(CC) -c -o $@ $<  $(CCFLAGS)

oraRecordFile.$(OBJEXT): oraRecordFile.cpp $(ORASMIHDR)
	$(CC) -c -o $@ $<  $(CCFLAGS)

oraKeyedFile.$(OBJEXT): oraKeyedFile.cpp $(ORASMIHDR)
	$(CC) -c -o $@ $< $(CCFLAGS)

oraRecord.$(OBJEXT): oraRecord.cpp $(ORASMIHDR)
	$(CC) -c -o $@ $<  $(CCFLAGS)

include ../include.dep

.PHONY: clean
clean:
	$(RM) -f *.o
	$(RM) -f *.lo
	$(RM) -f *.a
	$(RM) -f *.so
	$(RM) -f *.dll

