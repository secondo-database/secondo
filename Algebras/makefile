########################################################################
#
# SECONDO makefile for algebra manager and algebra modules
#
########################################################################

include ../makefile.config
ifdef platform
include ../makefile.$(platform)

.PHONY: all
all: makedirs specs

#specs:  Standard-C++/*.spec Relation-C++/*.spec StDb/*.spec
specs:  Standard-C++/*.spec Relation-C++/*.spec
	cat $^ > specs

.PHONY: makedirs
makedirs:
	$(MAKE) -C Management
	$(MAKE) -C Standard-C++
	$(MAKE) -C Function-C++
#	$(MAKE) -C Relation-C++
#	$(MAKE) -C StDb

.PHONY: clean
clean:
	$(RM) specs
	$(MAKE) -C Management clean
	$(MAKE) -C Standard-C++ clean
	$(MAKE) -C Function-C++ clean
#	$(MAKE) -C Relation-C++ clean

else
# Platform not specified
.DEFAULT:
	@echo *** Platform not specified in makefile.config
endif

