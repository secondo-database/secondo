########################################################################
#
# SECONDO makefile for the Storage Management Interface
#
########################################################################

include ../makefile.env

BDBOBJECTS=\
	smiEnvironment.$(OBJEXT) \
	smiKey.$(OBJEXT) \
	bdbKeyedFile.$(OBJEXT) \
	bdbRecordFile.$(OBJEXT) \
	bdbFile.$(OBJEXT) \
	bdbRecord.$(OBJEXT) \
	bdbEnvironment.$(OBJEXT)

ORAOBJECTS=\
	smiEnvironment.$(OBJEXT) \
	smiKey.$(OBJEXT) \
	oraKeyedFile.$(OBJEXT) \
	oraRecordFile.$(OBJEXT) \
	oraFile.$(OBJEXT) \
	oraRecord.$(OBJEXT) \
	oraEnvironment.$(OBJEXT)

SMIHDR = $(INCLUDEDIR)/SecondoConfig.h $(INCLUDEDIR)/SecondoSMI.h $(INCLUDEDIR)/SmiCodes.h $(INCLUDEDIR)/Profiles.h
ORASMIHDR = SmiORA.h $(SMIHDR)
BDBSMIHDR = SmiBDB.h $(SMIHDR)

.PHONY: all
ifeq ($(smitype),ora)
CCFLAGS= $(PICFLAG) $(OCICPPINCLUDE) $(DEFAULTCCFLAGS)

all: $(ORAOBJECTS)

buildlibs: $(LIBDIR)/libsmiora.$(LIBEXT)

else
CCFLAGS= $(PICFLAG) $(BDBINCLUDE) $(DEFAULTCCFLAGS)

all: $(BDBOBJECTS)

buildlibs: $(LIBDIR)/libsmibdb.$(LIBEXT)
endif


# --- Storage Management Interface library ---

ifeq ($(shared),yes)
# ... as shared object
ifeq ($(platform),win32)
ifeq ($(smitype),ora)
LDOPT = -Wl,--export-dynamic -Wl,--out-implib,$(LIBDIR)/libsmiora.$(LIBEXT).a
else
LDOPT = -Wl,--export-dynamic -Wl,--out-implib,$(LIBDIR)/libsmibdb.$(LIBEXT).a
endif
endif
endif

$(LIBDIR)/libsmibdb.$(LIBEXT): $(BDBOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/libsmibdb.$(LIBEXT) $(LDOPT) $(BDBOBJECTS) $(BDBLIBDIR) $(BDBLIB) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
ifeq ($(platform),win32)
	$(CP) $(LIBDIR)/libsmibdb.$(LIBEXT) $(BINDIR)/libsmibdb.$(LIBEXT)
endif
else
# ... as static library
	$(AR) -r $(LIBDIR)/libsmibdb.$(LIBEXT) $(BDBOBJECTS)
endif

$(LIBDIR)/libsmiora.$(LIBEXT): $(ORAOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/libsmiora.$(LIBEXT) $(LDOPT) $(ORAOBJECTS) $(OCICPPLIBDIR) $(OCICPPLIB) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
ifeq ($(platform),win32)
	$(CP) $(LIBDIR)/libsmiora.$(LIBEXT) $(BINDIR)/libsmiora.$(LIBEXT)
endif
else
# ... as static library
	$(AR) -r $(LIBDIR)/libsmiora.$(LIBEXT) $(ORAOBJECTS)
endif

# --- Storage Management Interface modules ---

smiEnvironment.$(OBJEXT): smiEnvironment.cpp $(SMIHDR) $(INCLUDEDIR)/Messenger.h $(INCLUDEDIR)/FileSystem.h
	$(CC) -c -o smiEnvironment.$(OBJEXT) smiEnvironment.cpp $(CCFLAGS)

smiKey.$(OBJEXT): smiKey.cpp $(SMIHDR)
	$(CC) -c -o smiKey.$(OBJEXT) smiKey.cpp $(CCFLAGS)

bdbEnvironment.$(OBJEXT): bdbEnvironment.cpp $(BDBSMIHDR)  $(INCLUDEDIR)/FileSystem.h
	$(CC) -c -o bdbEnvironment.$(OBJEXT) bdbEnvironment.cpp $(CCFLAGS)

bdbFile.$(OBJEXT): bdbFile.cpp $(BDBSMIHDR)
	$(CC) -c -o bdbFile.$(OBJEXT) bdbFile.cpp $(CCFLAGS)

bdbRecordFile.$(OBJEXT): bdbRecordFile.cpp $(BDBSMIHDR)
	$(CC) -c -o bdbRecordFile.$(OBJEXT) bdbRecordFile.cpp $(CCFLAGS)

bdbKeyedFile.$(OBJEXT): bdbKeyedFile.cpp $(BDBSMIHDR)
	$(CC) -c -o bdbKeyedFile.$(OBJEXT) bdbKeyedFile.cpp $(CCFLAGS)

bdbRecord.$(OBJEXT): bdbRecord.cpp $(BDBSMIHDR)
	$(CC) -c -o bdbRecord.$(OBJEXT) bdbRecord.cpp $(CCFLAGS)

oraEnvironment.$(OBJEXT): oraEnvironment.cpp $(ORASMIHDR)
	$(CC) -c -o oraEnvironment.$(OBJEXT) oraEnvironment.cpp $(CCFLAGS)

oraFile.$(OBJEXT): oraFile.cpp $(ORASMIHDR)
	$(CC) -c -o oraFile.$(OBJEXT) oraFile.cpp $(CCFLAGS)

oraRecordFile.$(OBJEXT): oraRecordFile.cpp $(ORASMIHDR)
	$(CC) -c -o oraRecordFile.$(OBJEXT) oraRecordFile.cpp $(CCFLAGS)

oraKeyedFile.$(OBJEXT): oraKeyedFile.cpp $(ORASMIHDR)
	$(CC) -c -o oraKeyedFile.$(OBJEXT) oraKeyedFile.cpp $(CCFLAGS)

oraRecord.$(OBJEXT): oraRecord.cpp $(ORASMIHDR)
	$(CC) -c -o oraRecord.$(OBJEXT) oraRecord.cpp $(CCFLAGS)

include ../include.dep

.PHONY: clean
clean:
	$(RM) -f *.o
	$(RM) -f *.lo
	$(RM) -f *.a
	$(RM) -f *.so
	$(RM) -f *.dll

