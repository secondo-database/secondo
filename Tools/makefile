########################################################################
#
# SECONDO makefile for tool and utility modules
#
########################################################################

include ../makefile.env

.PHONY: all
all: makedirs

.PHONY: makedirs
makedirs: makedirsnormal makedirsjni

.PHONY: makedirsnormal
makedirsnormal:
	$(MAKE) -C NestedLists
	$(MAKE) -C Parser
	$(MAKE) -C Utilities
	$(MAKE) -C pd 

.PHONY: makedirsjni
makedirsjni:
 ifeq ($(USE_JNI),"true")
	$(MAKE) -C Jni
 endif

.PHONY: clean
clean:
	$(MAKE) -C NestedLists clean
	$(MAKE) -C Parser clean
	$(MAKE) -C Utilities clean
	$(MAKE) -C Jni clean
	$(MAKE) -C pd clean

