########################################################################
#
# SECONDO makefile for the Storage Management Interface
#
########################################################################

include ../makefile.config
ifdef platform
include ../makefile.$(platform)

CCFLAGS= -I../StorageManager $(BDBINCLUDE) $(DEFAULTCCFLAGS)

.PHONY: all
all: testctable testnl testfilesystem testprofiles testsmi \
	testdynlib testsock1 testsock2 testsock3

# --- Storage Management Interface library ---

ifeq ($(shared),yes)
# ... as shared object
ifeq ($(platform),win32)
LDOPT = -Wl,--auto-import
endif
endif

# Test CTable module

testctable: tctable$(EXEEXT)
	$(CP) tctable$(EXEEXT) $(BINDIR)/tctable$(EXEEXT)

tctable$(EXEEXT): tctable.cpp $(INCLUDEDIR)/CTable.h $(INCLUDEDIR)/CTable.cpp
	$(CC) -o tctable$(EXEEXT) tctable.cpp  $(DEFAULTCCFLAGS) $(DEFAULTLIB)

# Test Nested List module

testnl: tnestedlist$(EXEEXT)
	$(CP) tnestedlist$(EXEEXT) $(BINDIR)/tnestedlist$(EXEEXT)

tnestedlist$(EXEEXT): tnestedlist.cpp $(INCLUDEDIR)/NestedList.h $(INCLUDEDIR)/CTable.h $(INCLUDEDIR)/CTable.cpp
	$(CC) -o tnestedlist$(EXEEXT) tnestedlist.cpp  $(DEFAULTCCFLAGS) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

# Test filesystem module

testfilesystem: tfilesystem$(EXEEXT)
	$(CP) tfilesystem$(EXEEXT) $(BINDIR)/tfilesystem$(EXEEXT)

tfilesystem$(EXEEXT): tfilesystem.o $(LIBDIR)/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tfilesystem$(EXEEXT) tfilesystem.o -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tfilesystem.o: tfilesystem.cpp $(INCLUDEDIR)/SecondoConfig.h $(INCLUDEDIR)/FileSystem.h
	$(CC) -c -o tfilesystem.o tfilesystem.cpp $(CCFLAGS)

# Test profiles module

testprofiles: tprofiles$(EXEEXT)
	$(CP) tprofiles$(EXEEXT) $(BINDIR)/tprofiles$(EXEEXT)

tprofiles$(EXEEXT): tprofiles.o $(LIBDIR)/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tprofiles$(EXEEXT) tprofiles.o -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tprofiles.o: tprofiles.cpp $(INCLUDEDIR)/Profiles.h
	$(CC) -c -o tprofiles.o tprofiles.cpp $(CCFLAGS)

# Test SMI module

testsmi: tsmi$(EXEEXT) tsmi3$(EXEEXT)
	$(CP) tsmi$(EXEEXT) $(BINDIR)/tsmi$(EXEEXT)

tsmi$(EXEEXT): tsmi.o $(LIBDIR)/libsmibdb.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tsmi$(EXEEXT) tsmi.o -L$(LIBDIR) $(BDBSMILIB) $(TOOLLIB) $(BDBLIBDIR) $(BDBLIB) $(DEFAULTLIB)

tsmi.o: tsmi.cpp $(INCLUDEDIR)/SecondoConfig.h $(INCLUDEDIR)/SecondoSMI.h ../StorageManager/SmiBDB.h
	$(CC) -c -o tsmi.o tsmi.cpp $(CCFLAGS)

tsmi3$(EXEEXT): tsmi3.o $(LIBDIR)/libsmibdb.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tsmi3$(EXEEXT) tsmi3.o -L$(LIBDIR) $(BDBSMILIB) $(TOOLLIB) $(BDBLIBDIR) $(BDBLIB) $(DEFAULTLIB)

tsmi3.o: tsmi3.cpp $(INCLUDEDIR)/SecondoConfig.h $(INCLUDEDIR)/SecondoSMI.h ../StorageManager/SmiBDB.h
	$(CC) -c -o tsmi3.o tsmi3.cpp $(CCFLAGS)

# Test Dynamic Library module

# ... Windows needs special treatment when creating DLLs
ifeq ($(platform),win32)
LDOPT1 = -Wl,--export-dynamic -Wl,--out-implib,libtdynlib1.$(DLLEXT).a
LDOPT2 = -Wl,--export-dynamic -Wl,--out-implib,libtdynlib2.$(DLLEXT).a
endif

testdynlib: tdynlib
	$(CP) tdlmain$(EXEEXT) $(BINDIR)/tdlmain$(EXEEXT)
	$(CP) libtdynlib1.$(DLLEXT) $(BINDIR)/libtdynlib1.$(DLLEXT)
	$(CP) libtdynlib2.$(DLLEXT) $(BINDIR)/libtdynlib2.$(DLLEXT)

tdynlib: tdlmain.o tdynlib1.$(DLLOBJEXT) tdynlib2.$(DLLOBJEXT) $(LIBDIR)/libsdbtool.$(LIBEXT)
	$(LD) $(DLLFLAGS) -o libtdynlib1.$(DLLEXT) $(LDOPT1) tdynlib1.$(DLLOBJEXT) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
	$(LD) $(DLLFLAGS) -o libtdynlib2.$(DLLEXT) $(LDOPT2) tdynlib2.$(DLLOBJEXT) -L. -ltdynlib1 -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
	$(LD) $(EXEFLAGS) -o tdlmain$(EXEEXT) tdlmain.o -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tdlmain.o: tdlmain.cpp $(INCLUDEDIR)/SecondoConfig.h $(INCLUDEDIR)/DynamicLibrary.h
	$(CC) -c -o tdlmain.o tdlmain.cpp $(CCFLAGS)

tdynlib1.$(DLLOBJEXT): tdynlib1.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib1.$(DLLOBJEXT) tdynlib1.cpp $(CCFLAGS)

tdynlib2.$(DLLOBJEXT): tdynlib2.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib2.$(DLLOBJEXT) tdynlib2.cpp $(CCFLAGS)

# Test Socket I/O module

testsock1: tsock1$(EXEEXT)
	$(CP) tsock1$(EXEEXT) $(BINDIR)/tsock1$(EXEEXT)

tsock1$(EXEEXT): tsock1.o $(LIBDIR)/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tsock1$(EXEEXT) tsock1.o -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tsock1.o: tsock1.cpp $(INCLUDEDIR)/SecondoConfig.h $(INCLUDEDIR)/SocketIO.h
	$(CC) -c -o tsock1.o tsock1.cpp $(CCFLAGS)

testsock2: tsock2$(EXEEXT)
	$(CP) tsock2$(EXEEXT) $(BINDIR)/tsock2$(EXEEXT)

tsock2$(EXEEXT): tsock2.o $(LIBDIR)/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tsock2$(EXEEXT) tsock2.o -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tsock2.o: tsock2.cpp $(INCLUDEDIR)/SecondoConfig.h $(INCLUDEDIR)/SocketIO.h
	$(CC) -c -o tsock2.o tsock2.cpp $(CCFLAGS)

testsock3: tsock3$(EXEEXT)
	$(CP) tsock3$(EXEEXT) $(BINDIR)/tsock3$(EXEEXT)

tsock3$(EXEEXT): tsock3.o $(LIBDIR)/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tsock3$(EXEEXT) tsock3.o -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tsock3.o: tsock3.cpp $(INCLUDEDIR)/SecondoConfig.h $(INCLUDEDIR)/SocketIO.h
	$(CC) -c -o tsock3.o tsock3.cpp $(CCFLAGS)

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo
	$(RM) *.a
	$(RM) *.so
	$(RM) *.dll

else
# Platform not specified
.DEFAULT:
	@echo *** Platform not specified in makefile.config
endif

