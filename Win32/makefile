########################################################################
#
# SECONDO makefile for Oracle OCI Win32 import library
#
########################################################################

include ../makefile.config
ifdef platform
include ../makefile.$(platform)
.SUFFIXES: .exe .res .a .o .c .cpp .cc .cxx .m .rc .p .f .F .r .y .l .s .S .def .h

OCIDLLHOME=$(ORACLEHOME)/bin
BDBDIR1=$(BDBDIR)-test

.PHONY: all
ifeq ($(smitype),ora) 
all: ../lib/libocicpp.a
else
all: ../lib/libdb32.a
endif

# --- Berkeley DB

../lib/libdb32.a:
	$(CP) BerkeleyDB/db.h $(BDBDIR1)/build_win32/db.h
	$(CP) BerkeleyDB/db_config.h $(BDBDIR1)/build_win32/db_config.h
	$(CP) BerkeleyDB/cxx_common.h $(BDBDIR1)/include/cxx_common.h
	$(CP) BerkeleyDB/libdb32.mingw $(BDBDIR1)/build_win32/libdb32.mingw
	$(CP) BerkeleyDB/os_rename.c $(BDBDIR1)/os_win32/os_rename.c
	make -C $(BDBDIR1)/build_win32 -f libdb32.mingw
	$(CP) $(BDBDIR1)/build_win32/libdb32.a ../lib/libdb32.a
	$(CP) $(BDBDIR1)/build_win32/libdb32.dll ../lib/libdb32.dll
	$(CP) $(BDBDIR1)/build_win32/libdb32.dll.a ../lib/libdb32.dll.a

# --- Oracle OCI

../lib/libocicpp.a: ../lib/liboci.dll.a
	$(CP) ocicpplib/ocicpp.h $(OCICPPDIR)/ocicpp/ocicpp.h
	$(CP) ocicpplib/libocicpp.mingw $(OCICPPDIR)/ocicpp/libocicpp.mingw
	$(CP) ocicpplib/demo-mingw.cpp $(OCICPPDIR)/demo/demo-mingw.cpp
	make -C $(OCICPPDIR)/ocicpp -f libocicpp.mingw
	$(CP) $(OCICPPDIR)/ocicpp/libocicpp.a ../lib/libocicpp.a
	$(CP) $(OCICPPDIR)/ocicpp/libocicpp.dll.a ../lib/libocicpp.dll.a

../lib/liboci.dll.a: oci.def
	dlltool -D oci.dll -d oci.def -l liboci.dll.a
	$(CP) liboci.dll.a ../lib/liboci.dll.a


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

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

