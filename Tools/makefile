########################################################################
#
# SECONDO makefile for tool and utility modules
#
########################################################################

include ../makefile.config
ifdef platform
include ../makefile.$(platform)

.PHONY: all
all: makedirs

.PHONY: makedirs
makedirs:
	$(MAKE) -C NestedLists
	$(MAKE) -C Parser
	$(MAKE) -C Utilities

.PHONY: clean
clean:
	$(MAKE) -C NestedLists clean
	$(MAKE) -C Parser clean
	$(MAKE) -C Utilities clean

else
# Platform not specified
.DEFAULT:
	@echo *** Platform not specified in makefile.config
endif

