include ../../makefile.env

INCLUDEFLAGS = -I. -I$(JNI_INCLUDE) -I$(JNI_MD_INCLUDE) $(PLINCLUDEFLAGS) -I$(INCLUDEDIR)

LINKFILES = $(JPL_LIB)/jpl.o $(SECONDOPL_DIR)/SecondoPL.o $(LIBDIR)/SecondoInterfaceCS.o $(LIBDIR)/SecondoInterfaceGeneral.o

TERMLIB=/usr/lib/zsh/4.0.6/zsh
#LDFLAGS= -L$(PL_LIB_DIR) -lpl -ldl -lreadline -L$(TERMCAP) -lm
#ADDITIONAL_FLAGS=-lsdbsys -lsdbtool -lsdbutils -lsmibdb -lsdbtool -lsdbsocket -lsdbutils -lsdbsocket  -ldb_cxx -ldb -lm -ldl \
#-lstdc++ -lpl -lreadline -lncurses -lm
#LD_OPTIMIZER_FLAGS=-L$(LIBDIR)  $(SECONDOPL_DIR)/DisplayTTY.o    $(ADDITIONAL_FLAGS)



.PHONY: all
all: compile build

.PHONY:compile
compile: $(JPL_LIB)/jpl.o

.PHONY:build
build: $(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT)


$(JPL_LIB)/jpl.o:
	$(CC) -c -fPIC -g -o $(JPL_LIB)/jpl.o $(INCLUDEFLAGS) jpl_fli_Prolog.c


#$(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT):
#	$(CC) -shared $(LINKFILES_ONE) -Wl,-soname,libjpl.so -o $(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT) $(LDFLAGS) $(LD_OPTIMIZER_FLAGS)

$(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT):
	$(CC) -shared $(EXEFLAGS) $(LDFLAGS) -Wl,-soname,$(SHARED_PREF)jpl.$(SHARED_EXT) -o $(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT) $(LINKFILES) $(LD_LINK_LIBS) $(PLLDFLAGS)

.PHONY:clean
clean:
	rm -f jpl_fli_Prolog.h
	rm -f $(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT)
