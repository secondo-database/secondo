include ../../makefile.env

INCLUDEFLAGS = -I. -I$(JNI_INCLUDE) -I$(JNI_MD_INCLUDE) $(PLINCLUDEFLAGS) -I$(INCLUDEDIR)

LINKFILES = $(JPL_LIB)/jpl.o $(SECONDOPL_DIR)/SecondoPL.o $(LIBDIR)/SecondoInterfaceCS.o $(LIBDIR)/SecondoInterfaceGeneral.o


.PHONY: all
all: compile build

.PHONY:compile
compile: $(JPL_LIB)/jpl.o

.PHONY:build
build: $(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT)


$(JPL_LIB)/jpl.o:
	$(CC) -c -fPIC -g -o $(JPL_LIB)/jpl.o $(INCLUDEFLAGS) jpl_fli_Prolog.c


$(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT):
	$(CC) -shared $(EXEFLAGS) $(LDFLAGS) -Wl,-soname,$(SHARED_PREF)jpl.$(SHARED_EXT)\
              -o $(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT) $(LINKFILES) $(LD_LINK_LIBS) $(PLLDFLAGS)

.PHONY:clean
clean:
	rm -f jpl_fli_Prolog.h
	rm -f $(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT)
