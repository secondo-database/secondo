########################################################################
#
# SECONDO makefile for the Storage Management Interface
#
########################################################################

include ../makefile.env



# Add the base name of your .cpp file here. The deafult rules should
# compile and link it, otherwise add special rules

# Nested List and Compact Tables
TESTNAMES += tctable tpctable tnestedlist

# Some Tools
TESTNAMES += tfilesystem tprofiles

# Storage Mangement Interface
TESTNAMES += tsmi tsmi3

# Socket Communication
TESTNAMES += tsock1 tsock2 tsock3

# Dynamic Library Module 
TESTNAMES += tdlmain

# Tool for Log Messages 
TESTNAMES += itlogmsg

# Persistent Array
#TESTNAMES += tparray

# Prefetch Iterator
#TESTNAMES += tprefetch

# Tuple Manager
#TESTNAMES += TupleTestBDB


TESTFILES := $(addsuffix $(EXEEXT), $(TESTNAMES)) libtdynlib1.$(DLLEXT) libtdynlib2.$(DLLEXT)

CODE_FILES =$(shell echo *.cpp)
CODE_DEPENDENCIES = $(patsubst %.cpp, %.dep, $(CODE_FILES))

ifndef file
OBJECTS = $(TESTFILES)
else
OBJECTS = $(file)$(EXEEXT)
endif


.PHONY: all 
all:  $(OBJECTS)

# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@


CCFLAGS := -I../StorageManager $(BDBINCLUDE) $(DEFAULTCCFLAGS) $(myflags)

# some default rules
%$(EXEEXT): %.cpp libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT) libsmibdb.$(LIBEXT) libsdbutils.$(LIBEXT)
	g++ -o $@ $< $(CCFLAGS) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(BDBSMILIB) $(BDBLIB) $(DEFAULTLIB)

%$(EXEEXT): %.o libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT) libsmibdb.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $< -L$(LIBDIR) $(BDBSMILIB) $(TOOLLIB) $(BDBLIBDIR) $(BDBLIB) $(DEFAULTLIB)

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)


# TupleManager

TupleTestBDB$(EXEEXT): SecondoTestFrame.o ../UserInterfaces/DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o $(TUPLEMANAGER) libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT)  
	$(LD) $(EXEFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)


# Test Dynamic Library module

# ... Windows needs special treatment when creating DLLs
ifeq ($(platform),win32)
LDOPT1 = -Wl,--export-dynamic -Wl,--out-implib,libtdynlib1.$(DLLEXT).a
LDOPT2 = -Wl,--export-dynamic -Wl,--out-implib,libtdynlib2.$(DLLEXT).a
endif

libtdynlib1.$(DLLEXT) libtdynlib2.$(DLLEXT): tdlmain.o tdynlib1.$(DLLOBJEXT) tdynlib2.$(DLLOBJEXT) libsdbtool.$(LIBEXT)
	$(LD) $(DLLFLAGS) -o libtdynlib1.$(DLLEXT) $(LDOPT1) tdynlib1.$(DLLOBJEXT) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
	$(LD) $(DLLFLAGS) -o libtdynlib2.$(DLLEXT) $(LDOPT2) tdynlib2.$(DLLOBJEXT) -L. -ltdynlib1 -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)


tdynlib1.$(DLLOBJEXT): tdynlib1.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib1.$(DLLOBJEXT) tdynlib1.cpp $(CCFLAGS)

tdynlib2.$(DLLOBJEXT): tdynlib2.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib2.$(DLLOBJEXT) tdynlib2.cpp $(CCFLAGS)


include $(CODE_DEPENDENCIES:.cpp=.dep)

.PHONY: clean
clean:
	$(RM) *.$(OBJEXT) $(OBJECTS) $(addprefix $(BINDIR)/,$(OBJECTS))
