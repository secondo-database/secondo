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


ifeq ($(smitype),ora)
CCFLAGS := $(PICFLAG) $(OCICPPINCLUDE) $(DEFAULTCCFLAGS)
ALLOBJ := $(ORAOBJECTS)
else
CCFLAGS := $(PICFLAG) $(BDBINCLUDE) $(DEFAULTCCFLAGS)
ALLOBJ := $(BDBOBJECTS)
endif

CODE_DEPENDENCIES = $(patsubst %.$(OBJEXT), %.dep, $(ALLOBJ))


.PHONY: all
all: $(CODE_DEPENDENCIES) $(ALLOBJ)

LIBNAME = libsmi$(SMILOW)
.PHONY: buildlibs
buildlibs: $(LIBNAME).$(LIBEXT)

# --- Storage Management Interface library ---

# Windows needs special treatment for dynamic libraries!
# The variable LIBNAME will be used in the LDOPTTOOL variable 
# which is only defined makefile.win32 included by makefile.env.

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

# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)

include $(CODE_DEPENDENCIES:.cpp=.dep)

.PHONY: clean
clean:
	$(RM) *.dep $(ALLOBJ) $(LIBDIR)/$(LIBNAME).$(LIBEXT)
