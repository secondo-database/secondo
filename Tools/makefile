########################################################################
#
# SECONDO makefile for tool and utility modules
#
########################################################################

include ../makefile.env

.PHONY: all
all: makedirs

.PHONY: makedirs
makedirs:
	$(MAKE) -C NestedLists
	$(MAKE) -C Parser
	$(MAKE) -C Utilities
	$(MAKE) -C TupleManager

.PHONY: clean
clean:
	$(MAKE) -C NestedLists clean
	$(MAKE) -C Parser clean
	$(MAKE) -C Utilities clean
	$(MAKE) -C TupleManager clean

