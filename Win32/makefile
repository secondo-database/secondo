########################################################################
#
# SECONDO makefile for Oracle OCI Win32 import library
#
########################################################################

include ../makefile.env

OCIDLLHOME=$(ORACLEHOME)/bin
BDB_BUILD_DIR=$(BDBDIR)/build_win32


# --- Berkeley DB
MODIFIED_BDB_FILES = \
		 db.h \
		 db_config.h \
		 cxx_common.h \
		 os_rename.c 

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
	$(CP) $(BDB_BUILD_DIR)/*.h $(BDBDIR)/include
	$(CP) $(BDBLIB_OBJECTS) $(LIBDIR)
	$(CP) $(addprefix $(BDB_BUILD_DIR)/, $(UTILITIES)) $(BUILDDIR)/bin

.PHONY: berkeley_db
$(BDBLIB_OBJECTS):
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
	$(RM) *.o
	$(RM) *.def
	$(RM) *.a
	$(RM) *.dll
	$(RM) ../

