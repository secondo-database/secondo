########################################################################
#
# SECONDO makefile for Oracle OCI Win32 import library
#
########################################################################

include ../makefile.env

OCIDLLHOME=$(ORACLEHOME)/bin
BDBDIR=$(BERKELEY_DB_DIR)
LIBDIR=$(SECONDO_BUILD_DIR)/lib

.PHONY: all
ifeq ($(smitype),ora) 
all: $(LIBDIR)/libocicpp.a
else
all: $(LIBDIR)/libdb32.a
endif

# --- Berkeley DB

$(LIBDIR)/libdb32.a:
	$(CP) BerkeleyDB/db.h $(BDBDIR)/build_win32/db.h
	$(CP) BerkeleyDB/db_config.h $(BDBDIR)/build_win32/db_config.h
	$(CP) BerkeleyDB/cxx_common.h $(BDBDIR)/include/cxx_common.h
	$(CP) BerkeleyDB/libdb32.mingw $(BDBDIR)/build_win32/libdb32.mingw
	$(CP) BerkeleyDB/os_rename.c $(BDBDIR)/os_win32/os_rename.c
	make -C $(BDBDIR)/build_win32 -f libdb32.mingw
	$(CP) $(BDBDIR)/build_win32/*.h $(BDBDIR)/include
	$(CP) $(BDBDIR)/build_win32/libdb32.a $(LIBDIR)/libdb32.a
	$(CP) $(BDBDIR)/build_win32/libdb32.dll $(LIBDIR)/libdb32.dll
	$(CP) $(BDBDIR)/build_win32/libdb32.dll.a $(LIBDIR)/libdb32.dll.a

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

