########################################################################
#
# SECONDO Makefile
#
########################################################################

# Include platform specification
include makefile.config

# Check whether platform is specified
ifdef platform

.PHONY: all
all: buildlib

.PHONY: buildlib
buildlib:
	$(MAKE) -C ClientServer
	$(MAKE) -C Tools
	$(MAKE) -C StorageManager

.PHONY: tests
tests: buildlib
	$(MAKE) -C Tests

#.PHONY: install
#install:
#	$(MAKE) -C lib install

#.PHONY: save
#save:
#	tar cvf SECONDO_`date +%d%m%y`.tar $(LISTSANDTABLESSRC) $(NESTEDLISTSSRC) $(PARSERSRC) $(TUPLEMANAGERSRC) $(ALGEBRAMANAGERSRC) $(QUERYPROCESSORSRC) $(TESTSSRC) $(CLIENTSERVERSRC) $(USERINTERFACESSRC) $(MAKEFILESRC) $(SPECPARSERSRC) $(STORAGEMANAGERSRC) $(ALGEBRAMODULESRC) $(INCLUDEDIR) $(DOCUMENTSDIR) $(FIGURES)

.PHONY: clean
clean:
	$(MAKE) -C ClientServer clean
	$(MAKE) -C StorageManager clean
	$(MAKE) -C Tools clean

.PHONY: clean_tests
clean_tests:
	$(MAKE) -C Tests clean

.PHONY: clean_all
clean_all: clean clean_tests

.PHONY: distclean
distclean:
	$(MAKE) -C ClientServer distclean
	$(MAKE) -C StorageManager distclean
	$(MAKE) -C Tools distclean

usage:
	@echo *** Usage of the SECONDO makefile:
	@echo ***
	@echo *** make [shared=yes] [target]
	@echo ***
	@echo *** The optional definition shared=yes specifies
	@echo *** that shared libraries are to be built. The
	@echo *** default is to build static libraries.
	@echo ***
	@echo *** The default target is <all>.

else
# Platform NOT specified in make configuration file
usage:
	@echo *** Usage of the SECONDO makefile:
	@echo ***
	@echo *** In the make configuration file makefile.config
	@echo *** exactly one platform definition must be uncommented:
	@echo ***
	@echo *** platform: win32    - Build SECONDO for 32-Bit-Windows
	@echo ***           linux    - Build SECONDO for Linux
	@echo ***           solaris  - Build SECONDO for Solaris
endif

