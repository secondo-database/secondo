########################################################################
#
# SECONDO makefile for algebra manager and algebra modules
#
########################################################################

include ../makefile.env

ALL_ALGEBRA_DIRS := $(ALGEBRA_DIRS)
ifeq ($(USE_JNI),"true")
ALL_ALGEBRA_DIRS += $(JNI_ALGEBRA_DIRS)
endif


.PHONY: all
all: makedirs specs

LS_COMMAND = $(addsuffix /*.spec, $(ALL_ALGEBRA_DIRS) )
SPECFILES = $(shell ls $(LS_COMMAND))

specs:  $(SPECFILES)
	chmod u+x *.awk
	cat $^ | sort -b | ./specs.awk
	cat $^ | ./rm_comment_spec.awk > specs


.PHONY: makedirs
makedirs:
	for dir in $(ALL_ALGEBRA_DIRS); do \
          if !($(MAKE) -C $$dir) then \
            exit 1; \
          fi \
        done


.PHONY:buildlibs
buildlibs:
	for dir in $(ALL_ALGEBRA_DIRS); do \
          if !($(MAKE) -C $$dir) then \
            exit 1; \
          fi \
        done


.PHONY: clean
clean:
	$(RM) specs
	for dir in $(ALL_ALGEBRA_DIRS); do \
          if !($(MAKE) -C $$dir clean) then \
            exit 1; \
          fi \
        done
