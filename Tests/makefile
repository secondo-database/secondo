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
	$(CP) tctable$(EXEEXT) ../bin/tctable$(EXEEXT)

tctable$(EXEEXT): tctable.cpp ../include/CTable.h ../include/CTable.cpp
	$(CC) -o tctable$(EXEEXT) tctable.cpp  $(DEFAULTCCFLAGS) $(DEFAULTLIB)

# Test Nested List module

testnl: tnestedlist$(EXEEXT)
	$(CP) tnestedlist$(EXEEXT) ../bin/tnestedlist$(EXEEXT)

tnestedlist$(EXEEXT): tnestedlist.cpp ../include/NestedList.h ../include/CTable.h ../include/CTable.cpp
	$(CC) -o tnestedlist$(EXEEXT) tnestedlist.cpp  $(DEFAULTCCFLAGS) $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

# Test filesystem module

testfilesystem: tfilesystem$(EXEEXT)
	$(CP) tfilesystem$(EXEEXT) ../bin/tfilesystem$(EXEEXT)

tfilesystem$(EXEEXT): tfilesystem.o ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tfilesystem$(EXEEXT) tfilesystem.o $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tfilesystem.o: tfilesystem.cpp ../include/SecondoConfig.h ../include/FileSystem.h
	$(CC) -c -o tfilesystem.o tfilesystem.cpp $(CCFLAGS)

# Test profiles module

testprofiles: tprofiles$(EXEEXT)
	$(CP) tprofiles$(EXEEXT) ../bin/tprofiles$(EXEEXT)

tprofiles$(EXEEXT): tprofiles.o ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tprofiles$(EXEEXT) tprofiles.o $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tprofiles.o: tprofiles.cpp ../include/Profiles.h
	$(CC) -c -o tprofiles.o tprofiles.cpp $(CCFLAGS)

# Test SMI module

testsmi: tsmi$(EXEEXT) tsmi3$(EXEEXT)
	$(CP) tsmi$(EXEEXT) ../bin/tsmi$(EXEEXT)

tsmi$(EXEEXT): tsmi.o ../lib/libsmibdb.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tsmi$(EXEEXT) tsmi.o $(LIBDIR) $(BDBSMILIB) $(TOOLLIB) $(BDBLIBDIR) $(BDBLIB) $(DEFAULTLIB)

tsmi.o: tsmi.cpp ../include/SecondoConfig.h ../include/SecondoSMI.h ../StorageManager/SmiBDB.h
	$(CC) -c -o tsmi.o tsmi.cpp $(CCFLAGS)

tsmi3$(EXEEXT): tsmi3.o ../lib/libsmibdb.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tsmi3$(EXEEXT) tsmi3.o $(LIBDIR) $(BDBSMILIB) $(TOOLLIB) $(BDBLIBDIR) $(BDBLIB) $(DEFAULTLIB)

tsmi3.o: tsmi3.cpp ../include/SecondoConfig.h ../include/SecondoSMI.h ../StorageManager/SmiBDB.h
	$(CC) -c -o tsmi3.o tsmi3.cpp $(CCFLAGS)

# Test Dynamic Library module

# ... Windows needs special treatment when creating DLLs
ifeq ($(platform),win32)
LDOPT1 = -Wl,--export-dynamic -Wl,--out-implib,libtdynlib1.$(DLLEXT).a
LDOPT2 = -Wl,--export-dynamic -Wl,--out-implib,libtdynlib2.$(DLLEXT).a
endif

testdynlib: tdynlib
	$(CP) tdlmain$(EXEEXT) ../bin/tdlmain$(EXEEXT)
	$(CP) libtdynlib1.$(DLLEXT) ../bin/libtdynlib1.$(DLLEXT)
	$(CP) libtdynlib2.$(DLLEXT) ../bin/libtdynlib2.$(DLLEXT)

tdynlib: tdlmain.o tdynlib1.$(DLLOBJEXT) tdynlib2.$(DLLOBJEXT) ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(DLLFLAGS) -o libtdynlib1.$(DLLEXT) $(LDOPT1) tdynlib1.$(DLLOBJEXT) $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
	$(LD) $(DLLFLAGS) -o libtdynlib2.$(DLLEXT) $(LDOPT2) tdynlib2.$(DLLOBJEXT) -L. -ltdynlib1 $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
	$(LD) $(EXEFLAGS) -o tdlmain$(EXEEXT) tdlmain.o $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tdlmain.o: tdlmain.cpp ../include/SecondoConfig.h ../include/DynamicLibrary.h
	$(CC) -c -o tdlmain.o tdlmain.cpp $(CCFLAGS)

tdynlib1.$(DLLOBJEXT): tdynlib1.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib1.$(DLLOBJEXT) tdynlib1.cpp $(CCFLAGS)

tdynlib2.$(DLLOBJEXT): tdynlib2.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib2.$(DLLOBJEXT) tdynlib2.cpp $(CCFLAGS)

# Test Socket I/O module

testsock1: tsock1$(EXEEXT)
	$(CP) tsock1$(EXEEXT) ../bin/tsock1$(EXEEXT)

tsock1$(EXEEXT): tsock1.o ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tsock1$(EXEEXT) tsock1.o $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tsock1.o: tsock1.cpp ../include/SecondoConfig.h ../include/SocketIO.h
	$(CC) -c -o tsock1.o tsock1.cpp $(CCFLAGS)

testsock2: tsock2$(EXEEXT)
	$(CP) tsock2$(EXEEXT) ../bin/tsock2$(EXEEXT)

tsock2$(EXEEXT): tsock2.o ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tsock2$(EXEEXT) tsock2.o $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tsock2.o: tsock2.cpp ../include/SecondoConfig.h ../include/SocketIO.h
	$(CC) -c -o tsock2.o tsock2.cpp $(CCFLAGS)

testsock3: tsock3$(EXEEXT)
	$(CP) tsock3$(EXEEXT) ../bin/tsock3$(EXEEXT)

tsock3$(EXEEXT): tsock3.o ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o tsock3$(EXEEXT) tsock3.o $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tsock3.o: tsock3.cpp ../include/SecondoConfig.h ../include/SocketIO.h
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

