########################################################################
#
# SECONDO makefile for Oracle OCI Win32 import library and Berkeley DB
#
########################################################################

CP := cp
RM := rm
CC := gcc

OCIDLLHOME=$(ORACLEHOME)/bin
BDB_BUILD_DIR=$(SECONDO_SDK)/db-4.1.25/build_win32

LIBDIR := $(SECONDO_SDK)/lib
INCDIR := $(SECONDO_SDK)/include
BINDIR := $(SECONDO_SDK)/bin

# --- Berkeley DB some files were modified in order to translate
#     it with GCC. For Documentation of changes see directory Berkeley
MODIFIED_BDB_FILES = \
		 db.h \
		 db_config.h \
		 cxx_common.h \
		 rijndael-api-fst.h 

MAKEFILE_BDB = libdb32.mingw
MAKEFILE_OCI = libocicpp.mingw

NEW_BDB_FILES = $(addprefix BerkeleyDB/, $(MODIFIED_BDB_FILES) $(MAKEFILE_BDB))

BDBLIB_FILES = \
	libdb32.dll \
	libdb32.a \
	libdb32.dll.a

BDBLIB_OBJECTS = $(addprefix $(BDB_BUILD_DIR)/, $(BDBLIB_FILES))


UTILITIES =\
	db_archive.exe\
	db_checkpoint.exe\
	db_deadlock.exe\
	db_dump.exe\
	db_load.exe\
	db_printlog.exe\
	db_recover.exe\
	db_stat.exe\
	db_upgrade.exe\
	db_verify.exe



ifeq ($(smitype),ora) 
OBJECTS :=  $(LIBDIR)/libocicpp.a
else
OBJECTS := $(BDBLIB_OBJECTS) utilities
endif

.PHONY: all
all: $(OBJECTS)


.PHONY: utilities
utilities:
	make -C $(BDB_BUILD_DIR) -f $(MAKEFILE_BDB) utilities


.PHONY: install
install:
	install -m744 $(BDB_BUILD_DIR)/*.h $(INCDIR)
	install -m744 $(BDB_BUILD_DIR)/../dbinc/*.h $(INCDIR)
	install -m755 $(BDBLIB_OBJECTS) $(LIBDIR)
	install -m755 $(addprefix $(BDB_BUILD_DIR)/, $(UTILITIES)) $(BINDIR) 

.PHONY: berkeley_db
$(BDBLIB_OBJECTS):
	chmod -R u+w $(BDB_BUILD_DIR)
	$(CP) $(NEW_BDB_FILES) $(BDB_BUILD_DIR)
	make -C $(BDB_BUILD_DIR) -f $(MAKEFILE_BDB)


# --- Oracle OCI
$(LIBDIR)/libocicpp.a: $(LIBDIR)/liboci.dll.a
	$(CP) ocicpplib/ocicpp.h $(OCICPPDIR)/ocicpp/ocicpp.h
	$(CP) ocicpplib/libocicpp.mingw $(OCICPPDIR)/ocicpp/libocicpp.mingw
	$(CP) ocicpplib/demo-mingw.cpp $(OCICPPDIR)/demo/demo-mingw.cpp
	make -C $(OCICPPDIR)/ocicpp -f libocicpp.mingw
	$(CP) $(OCICPPDIR)/ocicpp/libocicpp.a $(LIBDIR)/libocicpp.a
#	$(CP) $(OCICPPDIR)/ocicpp/libocicpp.dll $(LIBDIR)/libocicpp.dll
#	$(CP) $(OCICPPDIR)/ocicpp/libocicpp.dll.a $(LIBDIR)/libocicpp.dll.a

$(LIBDIR)/liboci.dll.a: oci.def
	dlltool -D oci.dll -d oci.def -l liboci.dll.a
	$(CP) liboci.dll.a $(LIBDIR)/liboci.dll.a


impdef.exe: impdef.cpp
	$(CC) -o impdef.exe impdef.cpp -s -lstdc++

oci.def: $(OCIDLLHOME)/oci.dll impdef.exe
	impdef $(OCIDLLHOME)/oci.dll >oci.def

.PHONY: clean
clean:
	$(RM) *.o *.def *.a *.dll
