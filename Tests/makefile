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

ifdef file
TESTFILES := $(file)
else
TESTFILES := $(TESTNAMES)
endif

CODE_FILES := $(addsuffix .cpp, $(TESTFILES))
CODE_DEPENDENCIES := $(addsuffix .dep, $(TESTFILES))

OBJECTS := $(addsuffix $(EXEEXT), $(TESTFILES))
ifndef file
OBJECTS += libtdynlib1.$(DLLEXT) libtdynlib2.$(DLLEXT)
endif


.PHONY: all 
all:  $(OBJECTS)
	echo $(OBJECTS)

# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@


CCFLAGS := -I../StorageManager $(BDBINCLUDE) $(DEFAULTCCFLAGS) $(myflags)

# some default rules
%$(EXEEXT): %.cpp libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT) libsmibdb.$(LIBEXT) libsdbutils.$(LIBEXT)
	g++ -o $@ $< $(CCFLAGS) $(LD_LINK_LIBS_ALG) 

%$(EXEEXT): %.o libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT) libsmibdb.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $< $(LD_LINK_LIBS)

%.$(OBJEXT): %.cpp
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


tdynlib1.$(DLLOBJEXT): tdynlib1.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib1.$(DLLOBJEXT) tdynlib1.cpp $(CCFLAGS)

tdynlib2.$(DLLOBJEXT): tdynlib2.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib2.$(DLLOBJEXT) tdynlib2.cpp $(CCFLAGS)


include $(CODE_DEPENDENCIES:.cpp=.dep)

.PHONY: clean
clean:
	$(RM) *.dep *.$(OBJEXT) $(OBJECTS) $(addprefix $(BINDIR)/,$(OBJECTS))
