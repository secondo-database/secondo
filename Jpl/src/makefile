include ../../makefile.env

INCLUDEFLAGS = -I. -I$(JNI_INCLUDE) -I$(JNI_MD_INCLUDE) $(PLINCLUDEFLAGS) -I$(INCLUDEDIR)

LINKFILES_ONE = $(JPL_LIB)/jpl.o $(SECONDOPL_DIR)/SecondoPL.o

TERMLIB=/usr/lib/zsh/4.0.6/zsh
LDFLAGS= -L$(PL_LIB_DIR) -lpl -ldl -lreadline -L$(TERMCAP) -lm
ADDITIONAL_FLAGS=-lsdbsys -lsdbtool -lsdbutils -lsmibdb -lsdbtool -lsdbsocket -lsdbutils -lsdbsocket  -ldb_cxx -ldb -lm -ldl \
-lstdc++ -lpl -lreadline -lncurses -lm
LD_OPTIMIZER_FLAGS=-L$(LIBDIR) $(LIBDIR)/SecondoInterfaceCS.o $(SECONDOPL_DIR)/DisplayTTY.o $(LIBDIR)/SecondoInterfaceGeneral.o   $(ADDITIONAL_FLAGS)



.PHONY: all
all: compile build

.PHONY:compile
compile: $(JPL_LIB)/jpl.o

.PHONY:build
build: $(OPTDIR)/libjpl.so


$(JPL_LIB)/jpl.o:
	$(CC) -c -fPIC -g -o $(JPL_LIB)/jpl.o $(INCLUDEFLAGS) jpl_fli_Prolog.c


$(OPTDIR)/libjpl.so:
	$(CC) -shared $(LINKFILES_ONE) -Wl,-soname,libjpl.so -o $(OPTDIR)/libjpl.so  $(LDFLAGS) $(LD_OPTIMIZER_FLAGS)



.PHONY:clean
clean:
	rm -f jpl_fli_Prolog.h
	rm -f $(OPTDIR)/libjpl.so
