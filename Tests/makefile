########################################################################
#
# SECONDO makefile for the Storage Management Interface
#
########################################################################

include ../makefile.env

CCFLAGS := -I../StorageManager $(BDBINCLUDE) $(DEFAULTCCFLAGS)

TESTRULES := \
	tctable\
	tfilesystem\
	tprofiles\
	tsmi\
	tsock1\
	tsock2\
	tsock3\
	TupleTestBDB\
	tdlmain\
	tnlcopy
	# tnl
TESTFILES := $(addsuffix $(EXEEXT), $(TESTRULES)) libtdynlib1.$(DLLEXT) libtdynlib2.$(DLLEXT)

ifndef file
OBJECTS = $(TESTFILES)
else
OBJECTS = $(file)$(EXEEXT)
endif


.PHONY: all 
all:  $(OBJECTS)

.PHONY: copy
copy: $(OBJECTS)
	$(CP) $(OBJECTS) $(BINDIR) 

	
$(file)$(EXEEXT) : $(file).cpp libsdbtool.$(LIBEXT)
	$(CC) -o $@ $<  $(DEFAULTCCFLAGS) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB) 

# --- Storage Management Interface library ---

ifeq ($(shared),yes)
# ... as shared object
ifeq ($(platform),win32)
LDOPT = -Wl,--auto-import
endif
endif

# TupleManager
SecondoTestFrame.o: SecondoTestFrame.cpp Application.h Profiles.h FileSystem.h SecondoSystem.h SecondoInterface.h SecondoSMI.h NestedList.h DisplayTTY.h
	$(CC) -c -o $@ $< $(CCFLAGS)

TupleTestBDB$(EXEEXT): SecondoTestFrame.o ../UserInterfaces/DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o $(TUPLEMANAGER) libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT)  
	$(LD) $(EXEFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)

# Test CTable module

tctable$(EXEEXT): tctable.cpp CTable.h CTable.cpp
	$(CC) -o $@ $<  $(DEFAULTCCFLAGS) $(DEFAULTLIB)

# Test Nested List module

tnestedlist$(EXEEXT): tnestedlist.cpp NestedList.h CTable.h CTable.cpp
	$(CC) -o $@ $<  $(DEFAULTCCFLAGS) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

# Test filesystem module

tfilesystem$(EXEEXT): tfilesystem.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $< -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tfilesystem.o: tfilesystem.cpp SecondoConfig.h FileSystem.h
	$(CC) -c -o  $@ $<  $(CCFLAGS)

# Test profiles module

tprofiles$(EXEEXT): tprofiles.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $< -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tprofiles.o: tprofiles.cpp Profiles.h
	$(CC) -c -o $@ $< $(CCFLAGS)

# Test SMI module

tsmi$(EXEEXT): tsmi.o libsmibdb.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $< -L$(LIBDIR) $(BDBSMILIB) $(TOOLLIB) $(BDBLIBDIR) $(BDBLIB) $(DEFAULTLIB)

tsmi.o: tsmi.cpp SecondoConfig.h SecondoSMI.h ../StorageManager/SmiBDB.h
	$(CC) -c -o $@ $< $(CCFLAGS)

tsmi3$(EXEEXT): tsmi3.o libsmibdb.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $< -L$(LIBDIR) $(BDBSMILIB) $(TOOLLIB) $(BDBLIBDIR) $(BDBLIB) $(DEFAULTLIB)

tsmi3.o: tsmi3.cpp SecondoConfig.h SecondoSMI.h ../StorageManager/SmiBDB.h
	$(CC) -c -o $@ $< $(CCFLAGS)

# Test Dynamic Library module

# ... Windows needs special treatment when creating DLLs
ifeq ($(platform),win32)
LDOPT1 = -Wl,--export-dynamic -Wl,--out-implib,libtdynlib1.$(DLLEXT).a
LDOPT2 = -Wl,--export-dynamic -Wl,--out-implib,libtdynlib2.$(DLLEXT).a
endif

libtdynlib1.$(DLLEXT) libtdynlib2.$(DLLEXT): tdlmain.o tdynlib1.$(DLLOBJEXT) tdynlib2.$(DLLOBJEXT) libsdbtool.$(LIBEXT)
	$(LD) $(DLLFLAGS) -o libtdynlib1.$(DLLEXT) $(LDOPT1) tdynlib1.$(DLLOBJEXT) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
	$(LD) $(DLLFLAGS) -o libtdynlib2.$(DLLEXT) $(LDOPT2) tdynlib2.$(DLLOBJEXT) -L. -ltdynlib1 -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)


tdlmain$(EXEEXT): tdlmain.o
	$(LD) $(EXEFLAGS) -o $@ $< -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tdlmain.o: tdlmain.cpp SecondoConfig.h DynamicLibrary.h
	$(CC) -c -o $@ $< $(CCFLAGS)

tdynlib1.$(DLLOBJEXT): tdynlib1.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib1.$(DLLOBJEXT) tdynlib1.cpp $(CCFLAGS)

tdynlib2.$(DLLOBJEXT): tdynlib2.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib2.$(DLLOBJEXT) tdynlib2.cpp $(CCFLAGS)

# Test Socket I/O module

tsock1$(EXEEXT): tsock1.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $< -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tsock1.o: tsock1.cpp SecondoConfig.h SocketIO.h
	$(CC) -c -o $@ $< $(CCFLAGS)

tsock2$(EXEEXT): tsock2.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $< -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tsock2.o: tsock2.cpp SecondoConfig.h SocketIO.h
	$(CC) -c -o $@ $< $(CCFLAGS)

tsock3$(EXEEXT): tsock3.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $< -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

tsock3.o: tsock3.cpp SecondoConfig.h SocketIO.h
	$(CC) -c -o $@ $<  $(CCFLAGS)

.PHONY: clean
clean:
	$(RM) *.o *.lo *.a *.so *.dll $(OBJECTS) $(addprefix $(BINDIR)/,$(OBJECTS))
